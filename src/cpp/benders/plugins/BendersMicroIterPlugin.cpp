/*
 * BendersMicroIterPlugin.cpp
 *
 * Implementation of BendersMicroIterPlugin: the C++ adapter that wraps an
 * IMicroIterationOracle and implements the BendersPlugin callback interface.
 */

#include "antares-xpansion/benders/plugins/BendersMicroIterPlugin.h"

#include <boost/serialization/vector.hpp>
#include <boost/tokenizer.hpp>
#include <chrono>
#include <fstream>
#include <sstream>
#include <stdexcept>


/* --------------------------------------------------------------------------
 * Construction
 * -------------------------------------------------------------------------- */

BendersMicroIterPlugin::BendersMicroIterPlugin(
    std::unique_ptr<IMicroIterationOracle> oracle,
    const SimulationOptions&               options,
    const CouplingMap&                     coupling_map,
    mpi::communicator*                     world,
    std::filesystem::path                  oracle_config_dir,
    bool                                   warm_start,
    std::filesystem::path                  constraints_dict_path)
    : oracle_(std::move(oracle))
    , options_(options)
    , coupling_map_(coupling_map)
    , world_(world)
    , oracle_config_dir_(std::move(oracle_config_dir))
    , warm_start_(warm_start)
    , constraints_dict_path_(std::move(constraints_dict_path))
{
    // Build subproblem -> constraint file mapping from the coupling map
    CouplingMap constraints_coupling_map_unused;
    CouplingMapGenerator::BuildSubProblemConstaintMap(
        coupling_map_,
        subproblem_constraint_map_,
        constraints_coupling_map_unused,
        options_);

    ReadConstraintsDictionary();
}


/* --------------------------------------------------------------------------
 * BendersPlugin: lifecycle
 * -------------------------------------------------------------------------- */

void BendersMicroIterPlugin::OnBendersStart(
    const SubproblemsMapPtr& subproblem_map,
    const Logger&             logger,
    const BendersBaseOptions& options,
    const SolverLogManager&   solver_log_manager)
{
    logger_ = logger;

    BuildConstraintsReaderMap(subproblem_map, options, solver_log_manager);

    // Collect subproblem IDs managed by this rank
    std::vector<std::string> subproblem_ids;
    subproblem_ids.reserve(subproblem_map.size());
    for (const auto& [sub_name, _] : subproblem_map)
        subproblem_ids.push_back(sub_name);

    // Collect master variable names from coupling_map ("master" entry)
    std::vector<std::string> master_var_names;
    auto master_it = coupling_map_.find("master");
    if (master_it != coupling_map_.end())
    {
        master_var_names.reserve(master_it->second.size());
        for (const auto& [var_name, _] : master_it->second)
            master_var_names.push_back(var_name);
    }

    oracle_->Initialize(subproblem_ids, master_var_names, oracle_config_dir_);

    micro_iterations_logger_ = std::make_shared<MicroIterationsLog>(
        options_,
        subproblem_constraint_map_,
        constraints_dict_,
        warm_start_,
        world_,
        options_.LOG_LEVEL);
}

void BendersMicroIterPlugin::OnBendersEnd()
{
    oracle_->Finalize();
}

void BendersMicroIterPlugin::OnBendersIterationStart()  {}
void BendersMicroIterPlugin::OnBendersIterationEnd()    {}
void BendersMicroIterPlugin::OnBendersSubResolutionStart() {}

void BendersMicroIterPlugin::OnBendersSubResolutionEnd(
    std::string sub_name, int num_micro_iter)
{
    if (options_.LOG_LEVEL >= 2)
        micro_iterations_logger_->AddMicroIterCount(sub_name, num_micro_iter);
}


/* --------------------------------------------------------------------------
 * BendersPlugin: master iteration boundary
 * -------------------------------------------------------------------------- */

void BendersMicroIterPlugin::OnBendersMasterResolutionStart(
    std::map<std::string, double>& master_out,
    int& num_iter)
{
    current_iteration_ = num_iter;

    // Reset per-master-iteration deduplication in cold-start mode.
    // In warm-start mode the deduplication set persists so already-active
    // rows are never re-added.
    if (!warm_start_)
    {
        for (auto& [sub, keys] : added_constraints_per_sub_)
            keys.clear();
    }

    auto t1 = std::chrono::high_resolution_clock::now();

    // Build a DecisionPoint from the master solution
    DecisionPoint master_dp(master_out.begin(), master_out.end());

    // Rank 0 computes the oracle state and serializes it
    OracleStateBytes state_bytes;
    if (world_->rank() == 0)
    {
        state_bytes = oracle_->UpdateStateFromMaster(master_dp, num_iter);
    }

    // Broadcast the serialized bytes to all ranks
    mpi::broadcast(*world_, state_bytes, 0);

    // All ranks (including rank 0) restore state from the broadcast bytes,
    // ensuring EvaluateViolatedConstraints behaves identically on every rank.
    oracle_->DeserializeState(state_bytes);

    auto t2 = std::chrono::high_resolution_clock::now();
    auto elapsed_us = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

    if (options_.LOG_LEVEL >= 2 && world_->rank() == 0)
        micro_iterations_logger_->AddMasterIterationLog(num_iter, std::to_string(elapsed_us));
}

void BendersMicroIterPlugin::OnBendersMasterResolutionEnd()
{
    if (!warm_start_)
    {
        for (auto& [sub_name, _] : added_constraints_per_sub_)
        {
            const std::string& constraint_file = subproblem_constraint_map_[sub_name];
            auto it = constraints_map_.find(constraint_file);
            if (it != constraints_map_.end())
                it->second->delete_added_rows();
        }
    }

    oracle_->OnMasterIterationEnd(current_iteration_);
}


/* --------------------------------------------------------------------------
 * BendersPlugin: micro-iteration boundary
 * -------------------------------------------------------------------------- */

void BendersMicroIterPlugin::OnBendersMicroIterationStart() {}

void BendersMicroIterPlugin::OnBendersMicroIterationEnd(
    std::string  sub_name,
    bool&        added_rows,
    std::string  solve_time)
{
    // Get the ConstraintsReader for this subproblem
    const std::string& constraint_file = subproblem_constraint_map_[sub_name];
    auto& constraint_reader = constraints_map_.at(constraint_file);

    // Read current subproblem primal solution
    std::vector<double> raw_solution = constraint_reader->get_sub_solution();

    // Build a full DecisionPoint from the solution vector.
    // We need the variable name -> column index map from the subproblem worker.
    // ConstraintsReader exposes get_variable_index_in_solution(id), but does not
    // expose the full name map directly. We therefore build the DecisionPoint by
    // iterating over the variables known to the subproblem via the coupling map.
    DecisionPoint sub_solution;
    auto coupling_it = coupling_map_.find(sub_name);
    if (coupling_it != coupling_map_.end())
    {
        for (const auto& [var_name, col_idx] : coupling_it->second)
        {
            if (col_idx >= 0 && static_cast<std::size_t>(col_idx) < raw_solution.size())
                sub_solution[var_name] = raw_solution[static_cast<std::size_t>(col_idx)];
        }
    }

    // Ask the oracle which constraints are violated at this solution
    auto t1 = std::chrono::high_resolution_clock::now();

    ConstraintKeySet violated_keys =
        oracle_->EvaluateViolatedConstraints(sub_name, sub_solution);

    // Add new (not yet active) constraints to the subproblem
    auto& active_keys = added_constraints_per_sub_[sub_name];
    std::vector<std::string> newly_added_keys;
    int new_rows_added = 0;

    for (const auto& key : violated_keys)
    {
        // Deduplication: skip if already active
        bool already_active = (std::find(active_keys.begin(), active_keys.end(), key)
                                != active_keys.end());
        if (already_active)
            continue;

        // Look up the MPS row names for this constraint key
        auto dict_it = constraints_dict_.find(key);
        if (dict_it == constraints_dict_.end())
            continue; // Unknown key; silently skip

        for (auto row_name : dict_it->second)
        {
            constraint_reader->add_rows(row_name);
            ++new_rows_added;
        }

        active_keys.push_back(key);
        newly_added_keys.push_back(key);
    }

    auto t2 = std::chrono::high_resolution_clock::now();
    auto elapsed_us = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

    if (options_.LOG_LEVEL >= 2)
    {
        micro_iterations_logger_->AddMicroIterionLog(
            sub_name,
            solve_time,
            std::to_string(elapsed_us),
            newly_added_keys);
    }

    added_rows = (new_rows_added > 0);
}


/* --------------------------------------------------------------------------
 * Private helpers
 * -------------------------------------------------------------------------- */

void BendersMicroIterPlugin::BuildConstraintsReaderMap(
    const SubproblemsMapPtr& subproblem_map,
    const BendersBaseOptions& options,
    const SolverLogManager&   solver_log_manager)
{
    for (const auto& [sub_name, sub_worker] : subproblem_map)
    {
        added_constraints_per_sub_[sub_name] = {};

        const std::string& constraints_file = subproblem_constraint_map_[sub_name];
        auto constraints_file_path =
            std::filesystem::path(options.INPUTROOT) / constraints_file;

        constraints_map_[constraints_file] = std::make_shared<ConstraintsReader>(
            constraints_file_path,
            options.SOLVER_NAME,
            solver_log_manager,
            logger_,
            options.LOG_LEVEL,
            options.PROBLEMS_FORMAT,
            sub_worker);
    }
}

void BendersMicroIterPlugin::ReadConstraintsDictionary()
{
    std::ifstream stream(constraints_dict_path_.c_str());
    if (!stream.is_open())
    {
        throw std::runtime_error(
            "BendersMicroIterPlugin: cannot open constraints dictionary: "
            + constraints_dict_path_.string());
    }

    typedef boost::tokenizer<boost::escaped_list_separator<char>> Tokenizer;
    std::string line;
    while (std::getline(stream, line))
    {
        Tokenizer tok(line);
        std::vector<std::string> tokens(tok.begin(), tok.end());
        if (tokens.empty())
            continue;

        const std::string& key = tokens[0];
        std::vector<std::string> row_names;
        if (tokens.size() > 1)
            row_names.assign(tokens.begin() + 1, tokens.end());

        constraints_dict_[key] = std::move(row_names);
    }
}

DecisionPoint BendersMicroIterPlugin::BuildDecisionPoint(
    const std::vector<double>& solution,
    const VariableMap&         name_to_id)
{
    DecisionPoint dp;
    for (const auto& [name, idx] : name_to_id)
    {
        if (idx >= 0 && static_cast<std::size_t>(idx) < solution.size())
            dp[name] = solution[static_cast<std::size_t>(idx)];
    }
    return dp;
}
