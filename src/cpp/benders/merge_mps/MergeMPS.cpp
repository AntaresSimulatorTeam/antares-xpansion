#include "antares-xpansion/benders/merge_mps/MergeMPS.h"

#include <filesystem>
#include <numeric>
#include <ranges>
#include <utility>

#include "antares-xpansion/benders/benders_core/CouplingMapGenerator.h"
#include "antares-xpansion/benders/merge_mps/JsonKeysConstants.h"
#include "antares-xpansion/benders/merge_mps/StandardLp.h"
#include "antares-xpansion/helpers/Timer.h"

AbstractMergeMPS::AbstractMergeMPS(MergeMPSOptions options,
                                   Logger logger,
                                   std::shared_ptr<Output::OutputWriter> writer):
    writer_(std::move(writer)),
    options_(std::move(options)),
    logger_(std::move(logger)),
    factory_()
{
    if (options_.SOLVER_NAME == "COIN")
    {
        options_.SOLVER_NAME = "CBC";
    }

    ptr_merged_solver_ = factory_.create_solver(options_.SOLVER_NAME);

    ptr_merged_solver_->set_output_log_level(options_.LOG_LEVEL);
}

/**
 * \brief Creates a local solver from a MPS file
 *
 * \param root_dir : Directory of MPS file
 *
 * \param filename : MPS file name
 */
SolverAbstract::Ptr AbstractMergeMPS::get_local_solver(const std::filesystem::path& root_dir,
                                                       const std::string& filename) const
{
    /**
     * Limitation: on windows may not support master problem with full path as name
     */
    SolverAbstract::Ptr ptr_solver = factory_.create_solver(options_.SOLVER_NAME);
    ptr_solver->set_output_log_level(options_.LOG_LEVEL);
    ptr_solver->read_prob_mps(root_dir / filename);
    return ptr_solver;
}

/**
 * \brief Weights local solver's objective function by a given value
 *
 * \param local_solver : Subproblem solver that will be modified
 *
 * \param weight : Factor to apply
 */
void AbstractMergeMPS::multiply_obj_by_weight_factor(SolverAbstract& local_solver,
                                                     double weight) const
{
    // Avoid unnecessary computation if weight is 1
    if (std::fabs(weight - 1.) <= 1.e-6)
    {
        return;
    }

    const int nb_cols{local_solver.get_ncols()};

    std::vector<int> indices(nb_cols);
    std::iota(indices.begin(), indices.end(), 0);

    std::vector<double> obj_coeff(nb_cols);
    solver_get_obj_func_coeffs(local_solver, obj_coeff, 0, nb_cols - 1);

    std::for_each(obj_coeff.begin(),
                  obj_coeff.end(),
                  [weight](double& coeff) { return coeff *= weight; });

    local_solver.chg_obj(indices, obj_coeff);
}

/**
 * \brief Merges local to global solver
 *
 * \param local_solver : Local solver
 *
 * \param local_prefix : Local solver's prefix
 *
 * \param local_var_map : Local solver's variable mapping
 *
 * \param filename : MPS file name
 */
VariableMap AbstractMergeMPS::merge_local_solver(SolverAbstract& local_solver,
                                                 const std::string& local_prefix,
                                                 const VariableMap& local_var_map,
                                                 const std::string& filename)
{
    VariableMap merged_var_map;
    StandardLp lpData(local_solver);

    // Prefix the name of the problem (Master and slaves alike)
    // along with the counting
    lpData.append_in(*ptr_merged_solver_, local_prefix);

    for (const auto& [var_name, var_idx]: local_var_map)
    {
        const int merged_col_index = ptr_merged_solver_->get_col_index(local_prefix + var_name);
        if (merged_col_index == -1)
        {
            const auto output_root = std::filesystem::path(options_.OUTPUTROOT);
            std::cerr << LOGLOCATION << "missing variable " << var_name << " in " << filename
                      << " supposedly renamed to " << local_prefix + var_name << ".";

            ptr_merged_solver_->write_prob_lp(output_root / "mergeError.lp");
            ptr_merged_solver_->write_prob_mps(output_root / ("mergeError" + MPS_SUFFIX));

            std::exit(1);
        }
        merged_var_map[var_name] = merged_col_index;
    }

    return merged_var_map;
}

/**
 * \brief Export problem into mps and lp files
 */
void AbstractMergeMPS::export_problem()
{
    const auto output_root = std::filesystem::path(options_.OUTPUTROOT);
    logger_->display_message("Problems merged.");
    logger_->display_message("Writing mps file");
    ptr_merged_solver_->write_prob_mps(output_root / ("log_merged" + MPS_SUFFIX));
    logger_->display_message("Writing lp file");
    ptr_merged_solver_->write_prob_lp(output_root / "log_merged.lp");
}

/**
 * \brief Merge and solve master and subproblems
 */
void MergeMasterSubproblemMPS::launch()
{
    build_problem();

    export_problem();

    const bool is_optimal = solve();

    output_solution(is_optimal);
}

/**
 * \brief Build merged problem
 */
void MergeMasterSubproblemMPS::build_problem()
{
    const auto root_dir = std::filesystem::path(options_.INPUTROOT);

    logger_->display_message("Merging master and subproblems...");

    const CouplingMap local_structure = CouplingMapGenerator::BuildInput(
      root_dir / options_.STRUCTURE_FILE,
      logger_.get(),
      "Merge mps");

    // TODO Investigate why following check
    // TODO creates a segfault when structure.txt is empty
    // if (local_structure.empty())
    // {
    //     logger_->display_message("Nothing to merge. Returning empty problem.");
    //     return;
    // }

    const int nb_sub_problems = local_structure.size() - 1;

    int current_prob_id{0};
    for (const auto& [filename, var_map]: local_structure)
    {
        SolverAbstract::Ptr ptr_solver = get_local_solver(root_dir, filename);

        // Change the weight of coeff in the objective function
        const double weight = get_problem_obj_weight(nb_sub_problems, filename);
        multiply_obj_by_weight_factor(*ptr_solver, weight);

        const std::string local_prefix = "prob" + std::to_string(current_prob_id++) + "_";
        structure_[filename] = merge_local_solver(*ptr_solver, local_prefix, var_map, filename);
    }

    add_coupling_constraints();
}

/**
 * \brief Solve merged problem
 *
 * \param merged_solver : Ref to merged solver
 *
 * \param nb_threads : Number of threads to use
 */
bool MergeMasterSubproblemMPS::solve(int nb_threads)
{
    ptr_merged_solver_->set_threads(nb_threads);

    logger_->display_message("Solving...");

    Timer timer;
    int status{0};

    if (ptr_merged_solver_->get_n_integer_vars() > 0)
    {
        status = ptr_merged_solver_->solve_mip();
    }
    else
    {
        status = ptr_merged_solver_->solve_lp();
    }

    logger_->log_total_duration(timer.elapsed());

    return status == SOLVER_STATUS::OPTIMAL;
}

/**
 * \brief Post-process and output solution
 *
 * \param merged_solver : Ref to merged solver
 *
 * \param structure : Mapping of files and investments candidates
 *
 * \param candidates : Mapping of investments candidates
 *
 * \param is_sol_optimal : Flag true if solution is optimal, false otherwise
 */
void MergeMasterSubproblemMPS::output_solution(bool is_sol_optimal)
{
    double overall_cost{0}, investment_cost{0}, operational_cost{0};

    std::vector<double> solution(ptr_merged_solver_->get_ncols()),
      obj_coeff(ptr_merged_solver_->get_ncols()), lb_values(ptr_merged_solver_->get_ncols()),
      ub_values(ptr_merged_solver_->get_ncols());

    if (ptr_merged_solver_->get_n_integer_vars() > 0)
    {
        overall_cost = ptr_merged_solver_->get_mip_value();
        ptr_merged_solver_->get_mip_sol(solution.data());
    }
    else
    {
        overall_cost = ptr_merged_solver_->get_lp_value();
        ptr_merged_solver_->get_lp_sol(solution.data(), nullptr, nullptr);
    }

    ptr_merged_solver_->get_obj(obj_coeff.data(), 0, ptr_merged_solver_->get_ncols() - 1);
    ptr_merged_solver_->get_lb(lb_values.data(), 0, ptr_merged_solver_->get_ncols() - 1);
    ptr_merged_solver_->get_ub(ub_values.data(), 0, ptr_merged_solver_->get_ncols() - 1);

    std::vector<Output::CandidateData> candidates;
    for (const auto& [var_name, var_idx]: structure_[options_.MASTER_NAME])
    {
        const auto& candidate = candidates.emplace_back(var_name,
                                                        solution[var_idx],
                                                        lb_values[var_idx],
                                                        ub_values[var_idx]);
        investment_cost += candidate.invest * obj_coeff[var_idx];
    }
    if (candidates.empty())
    {
        std::cerr << LOGLOCATION << "Could not find '" << options_.MASTER_NAME
                  << "' in structure\n";
    }

    operational_cost = overall_cost - investment_cost;

    Output::SolutionData sol_infos;
    sol_infos.nbWeeks_p = static_cast<int>(structure_.size());

    sol_infos.solution.lb = overall_cost;
    sol_infos.solution.ub = overall_cost;
    sol_infos.solution.investment_cost = investment_cost;
    sol_infos.solution.operational_cost = operational_cost;
    sol_infos.solution.overall_cost = overall_cost;

    sol_infos.solution.candidates.clear();
    sol_infos.solution.candidates.insert(sol_infos.solution.candidates.end(),
                                         std::make_move_iterator(candidates.begin()),
                                         std::make_move_iterator(candidates.end()));
    candidates.clear();

    sol_infos.problem_status = is_sol_optimal ? "OPTIMAL" : "ERROR";

    writer_->update_solution(sol_infos);
    writer_->dump();
}

/*!
 *  \brief Return subproblem weight value
 *
 *  \param nb_subproblems : total number of subproblems
 *
 *  \param name : subproblem name
 */
double MergeMasterSubproblemMPS::get_problem_obj_weight(int nb_subproblems,
                                                        const std::string& name) const
{
    if (options_.MASTER_NAME == name)
    {
        return 1.0;
    }
    if (options_.SLAVE_WEIGHT == SUBPROBLEM_WEIGHT_UNIFORM_CST_STR)
    {
        return 1.0 / nb_subproblems;
    }
    if (options_.SLAVE_WEIGHT == SUBPROBLEM_WEIGHT_CST_STR)
    {
        return 1.0 / options_.SLAVE_WEIGHT_VALUE;
    }
    const auto found = options_.weights.find(name);
    if (found == options_.weights.end())
    {
        logger_->display_message("No weight found for " + name
                                   + ". Problem will not contribute to objective function",
                                 LogUtils::LOGLEVEL::WARNING,
                                 "MergeMPS");
        return 0.;
    }
    return found->second;
}

/**
 * \brief Add coupling equality constraints between subproblems
 */
void MergeMasterSubproblemMPS::add_coupling_constraints()
{
    std::map<std::string, std::vector<int>> variables;
    for (const auto& [_, var_map]: structure_)
    {
        for (const auto& [var_name, var_idx]: var_map)
        {
            variables[var_name].push_back(var_idx);
        }
    }

    // Add n-1 new constraints per variable where n is
    // the number of problems where this variable appears
    // i.e. the number of columns in the merged problem
    // representing this variable
    const size_t nb_rows_reserve = std::accumulate(variables.cbegin(),
                                                   variables.cend(),
                                                   size_t{0},
                                                   [](size_t acc, const auto& pair)
                                                   {
                                                       const auto& indices = pair.second;
                                                       return acc + indices.size() - 1;
                                                   });
    const size_t nb_elem_reserve = 2 * nb_rows_reserve;

    logger_->display_message("About to add " + std::to_string(nb_rows_reserve)
                             + " coupling constraints");

    std::vector<int> mstart; // Constraints' offsets
    mstart.reserve(nb_rows_reserve + 1);

    std::vector<int> mclind;     // Variables' indices
    std::vector<double> dmatval; // Variables' values
    dmatval.reserve(nb_elem_reserve);
    mclind.reserve(nb_elem_reserve);

    int nb_rows{0};
    int nb_elem{0};
    for (const auto& [var_name, indices]: variables)
    {
        const int ref_var_idx = indices[0];

        // Starting from second element onwards
        // Add one equality constraint per pair of variables:
        // 1 * ref - 1 * second = 0
        for (int var_idx: indices | std::views::drop(1))
        {
            mstart.push_back(nb_elem);

            mclind.push_back(ref_var_idx);
            dmatval.push_back(1);
            ++nb_elem;

            mclind.push_back(var_idx);
            dmatval.push_back(-1);
            ++nb_elem;

            ++nb_rows;
        }

        logger_->display_message(var_name + " : " + std::to_string(indices.size() - 1)
                                 + " coupling constraints built");
    }
    mstart.push_back(nb_elem);

    std::vector<double> rhs(nb_rows, 0);    // Constraints' rhs
    std::vector<char> qrtype(nb_rows, 'E'); // Constraints' types

    solver_addrows(*ptr_merged_solver_, qrtype, rhs, {}, mstart, mclind, dmatval);
}

MergeMasterMasterMPS::PathwayCandidateProfile::PathwayCandidateProfile(const Json::Value& data)
{
    const auto& investment_data = data[JSON_KEY_INVESTMENT];
    investment = {investment_data[0].asDouble(),
                  investment_data[1].asDouble(),
                  investment_data[2].asDouble()};

    const auto& decommissioning_data = data[JSON_KEY_DECOMMISSIONING];
    decommissioning = {decommissioning_data[0].asDouble(),
                       decommissioning_data[1].asDouble(),
                       decommissioning_data[2].asDouble()};
}

MergeMasterMasterMPS::PathwayNode::PathwayNode(const std::string&& node, const Json::Value& data):
    name(node)
{
    // TODO improve parsing of json file
    path = std::filesystem::path(data[JSON_KEY_PATH].asString());

    if (data.isMember(JSON_KEY_PARENT))
    {
        parent = data[JSON_KEY_PARENT].asString();
    }

    weight = data[JSON_KEY_WEIGHT].asDouble();

    const auto& candidates_data = data[JSON_KEY_CANDIDATES];
    for (const auto& var_name: candidates_data.getMemberNames())
    {
        candidates[var_name].profile = candidates_data[var_name].asString();
    }
}

std::string MergeMasterMasterMPS::PathwayNode::get_candidate_full_name(
  const std::string& var_name) const
{
    return this->name + "." + var_name;
}

MergeMasterMasterMPS::MergeMasterMasterMPS(MergeMPSOptions options,
                                           Logger logger,
                                           std::shared_ptr<Output::OutputWriter> writer,
                                           const std::filesystem::path& tree_filename):
    AbstractMergeMPS(options, logger, writer)
{
    const auto raw_input = get_json_file_content(tree_filename);

    const auto& initial_capacities_data = raw_input[JSON_KEY_INITIAL_CAPACITIES];
    for (Json::String candidate_name: initial_capacities_data.getMemberNames())
    {
        initial_capacities_[candidate_name] = initial_capacities_data[candidate_name].asDouble();
    }

    const auto& candidate_profiles_data = raw_input[JSON_KEY_CANDIDATES_PROFILES];
    for (Json::String profile_name: candidate_profiles_data.getMemberNames())
    {
        const Json::Value& data = candidate_profiles_data[profile_name];
        candidate_profiles_[profile_name] = PathwayCandidateProfile(data);
    }

    const auto& tree_data = raw_input[JSON_KEY_TREE];
    for (Json::String tree_node: tree_data.getMemberNames())
    {
        const Json::Value& data = tree_data[tree_node];
        tree_.emplace_back(std::move(tree_node), data);
    }

    // TODO Add coherence check for parents?
}

/**
 * \brief Merge and master problems
 */
void MergeMasterMasterMPS::launch()
{
    build_problem();
    export_problem();
}

/**
 * \brief Build merged problem
 */
void MergeMasterMasterMPS::build_problem()
{
    const auto root_dir{std::filesystem::path(options_.INPUTROOT)};

    logger_->display_message("Merging master problems...");

    int current_prob_id{0};
    for (auto& tree_node: tree_)
    {
        const auto node_path{root_dir / tree_node.path};

        const CouplingMap local_structure = CouplingMapGenerator::BuildInput(
          node_path / options_.STRUCTURE_FILE,
          logger_.get(),
          "Merge Master mps");

        for (const auto& [filename, var_map]:
             local_structure
               | std::views::filter([this](const auto& pair)
                                    { return pair.first == options_.MASTER_NAME; }))
        {
            SolverAbstract::Ptr ptr_solver = get_local_solver(node_path, filename);

            // Zero out objective coefficients so only the incremental
            // variables can affect the final cost
            multiply_obj_by_weight_factor(*ptr_solver, 0.0);

            const std::string local_prefix = "prob" + std::to_string(current_prob_id++) + "_";
            for (const auto& [var_name, var_idx]:
                 merge_local_solver(*ptr_solver, local_prefix, var_map, filename))
            {
                tree_node.candidates[var_name].index = var_idx;
            }
        }
    }

    add_incremental_variables();
    add_coupling_constraints();
}

/**
 * \brief Add investment delta variables to merged master problem
 */
void MergeMasterMasterMPS::add_incremental_variables()
{
    // For each node, 2 variables per candidate
    const int nb_new_var_reserve = 2 * tree_.size() * tree_[0].candidates.size();

    std::vector<double> objx,          // Objective coefficients
      bdl,                             // Lower bounds
      bdu;                             // Upper bounds
    std::vector<char> colTypes;        // Type
    std::vector<std::string> colNames; // Names

    objx.reserve(nb_new_var_reserve);
    bdl.reserve(nb_new_var_reserve);
    bdu.reserve(nb_new_var_reserve);
    colTypes.reserve(nb_new_var_reserve);
    colNames.reserve(nb_new_var_reserve);

    int nb_vars = ptr_merged_solver_->get_ncols();
    for (auto& tree_node: tree_)
    {
        for (auto& [var_name, candidate]: tree_node.candidates)
        {
            const std::string var_full_name = tree_node.get_candidate_full_name(var_name);
            const PathwayCandidateProfile& profile = candidate_profiles_[candidate.profile];

            // dx_plus
            objx.push_back(profile.investment.obj * tree_node.weight);
            bdl.push_back(profile.investment.bdl);
            bdu.push_back(profile.investment.bdu);
            colTypes.push_back('C');
            colNames.push_back(var_full_name + "_dx_plus");
            candidate.dx_plus_index = ++nb_vars;

            // dx_minus
            objx.push_back(profile.decommissioning.obj * tree_node.weight);
            bdl.push_back(profile.decommissioning.bdl);
            bdu.push_back(profile.decommissioning.bdu);
            colTypes.push_back('C');
            colNames.push_back(var_full_name + "_dx_minus");
            candidate.dx_minus_index = ++nb_vars;
        }
    }

    std::vector<int> mstart(nb_new_var_reserve); // Offsets
    std::iota(mstart.begin(), mstart.end(), 0);

    solver_addcols(*ptr_merged_solver_, objx, mstart, {}, {}, bdl, bdu, colTypes, colNames);
}

/**
 * \brief Add coupling equality constraints between master problems
 */
void MergeMasterMasterMPS::add_coupling_constraints()
{
    // For each node, 1 constraint per candidate
    // For each constraint, 4 columns
    const size_t nb_rows_reserve = tree_.size() * tree_[0].candidates.size();
    const size_t nb_elem_reserve = 4 * nb_rows_reserve;

    std::vector<int> mclind;     // Variables' indices
    std::vector<double> dmatval; // Variables' values
    dmatval.reserve(nb_elem_reserve);
    mclind.reserve(nb_elem_reserve);

    std::vector<int> mstart;  // Constraints' offsets
    std::vector<double> rhs;  // Constraints' rhs
    std::vector<char> qrtype; // Constraints' types
    mstart.reserve(nb_rows_reserve + 1);
    rhs.reserve(nb_rows_reserve);
    qrtype.reserve(nb_rows_reserve);

    int nb_elem{0};
    for (const auto& tree_node: tree_)
    {
        const auto parent = tree_node.parent.has_value()
                              ? std::ranges::find(tree_,
                                                  tree_node.parent.value(),
                                                  &PathwayNode::name)
                              : tree_.end();

        for (const auto& [var_name, candidate]: tree_node.candidates)
        {
            const std::string var_full_name = tree_node.get_candidate_full_name(var_name);
            const PathwayCandidate& candidate = tree_node.candidates.at(var_name);
            const int parent_index = (parent != tree_.end()) ? parent->candidates.at(var_name).index
                                                             : -1;

            mstart.push_back(nb_elem);

            mclind.push_back(candidate.index);
            dmatval.push_back(1);
            ++nb_elem;

            mclind.push_back(candidate.dx_plus_index);
            dmatval.push_back(-1);
            ++nb_elem;

            mclind.push_back(candidate.dx_minus_index);
            dmatval.push_back(1);
            ++nb_elem;

            if (parent_index >= 0) [[likely]]
            {
                mclind.push_back(parent_index);
                dmatval.push_back(-1);
                rhs.push_back(initial_capacities_[var_name]); // If not found, defaults to 0
                ++nb_elem;
            }
            else
            {
                rhs.push_back(0.0);
            }

            qrtype.push_back('E');
        }
    }
    mstart.push_back(nb_elem);

    solver_addrows(*ptr_merged_solver_, qrtype, rhs, {}, mstart, mclind, dmatval);
}
