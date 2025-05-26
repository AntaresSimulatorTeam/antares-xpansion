#include "antares-xpansion/benders/merge_mps/MergeMPS.h"

#include <filesystem>
#include <fmt/format.h>
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
    factory_(),
    solver_io_()
{
    if (options_.SOLVER_NAME == "COIN")
    {
        options_.SOLVER_NAME = "CBC";
    }

    ptr_merged_solver_ = factory_.create_solver(options_.SOLVER_NAME);
    ptr_merged_solver_->set_output_log_level(options_.LOG_LEVEL);

    solver_io_.configure(options_.SOLVER_NAME, options_.PROBLEMS_FORMAT);
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

    solver_io_.read(ptr_solver.get(), root_dir / filename);

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
 * \brief Interrupt the program and output current merged .lp and .mps files
 *
 * \param message : Error message to be output
 */
void AbstractMergeMPS::terminate(const std::string& location, const std::string& message) const
{
    const auto output_root = std::filesystem::path(options_.OUTPUTROOT);
    std::cerr << location << message << std::endl;

    ptr_merged_solver_->write_prob_lp(output_root / "mergeError.lp");
    ptr_merged_solver_->write_prob_mps(output_root / ("mergeError" + MPS_SUFFIX));

    std::exit(1);
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
        // Local var name might not be the same as the var name in the CouplingMap
        // e.g. in the trajectory case, we add a prefix to var names in the master problem but
        // the subproblems are not modified
        const std::vector<std::string> local_var_names = local_solver.get_col_names(var_idx,
                                                                                    var_idx);
        if (local_var_names.size() != 1)
        {
            terminate(LOGLOCATION,
                      fmt::format("Found {} candidates for variable '{}' in {}.",
                                  local_var_names.size(),
                                  var_name,
                                  filename));
        }
        const std::string& local_var_name = local_var_names[0];

        // Find the variable in the merged solver.
        const std::string prefixed_var_name = local_prefix + local_var_name;
        const int merged_col_index = ptr_merged_solver_->get_col_index(prefixed_var_name);
        if (merged_col_index == -1)
        {
            terminate(LOGLOCATION,
                      fmt::format("Missing variable '{}' in {} supposedly renamed to '{}'.",
                                  var_name,
                                  filename,
                                  prefixed_var_name));
        }
        merged_var_map[var_name] = merged_col_index;
    }

    return merged_var_map;
}

/**
 * \brief Export problem into OUTPUTROOT/filename and optionally writes the lp variant
 */
void AbstractMergeMPS::export_problem(const std::string& filename, bool export_lp)
{
    const auto output_root = std::filesystem::path(options_.OUTPUTROOT);
    logger_->display_message("Problems merged.",
                             LogUtils::LOGLEVEL::INFO,
                             MERGE_MPS_LOGGER_CONTEXT);
    logger_->display_message("Writing merged file",
                             LogUtils::LOGLEVEL::INFO,
                             MERGE_MPS_LOGGER_CONTEXT);
    solver_io_.write(ptr_merged_solver_.get(), output_root / filename);

    if (export_lp)
    {
        logger_->display_message("Writing lp file",
                                 LogUtils::LOGLEVEL::INFO,
                                 MERGE_MPS_LOGGER_CONTEXT);
        ptr_merged_solver_->write_prob_lp(output_root / (filename + ".lp"));
    }
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

    logger_->display_message("Merging master and subproblems...",
                             LogUtils::LOGLEVEL::INFO,
                             MERGE_MPS_LOGGER_CONTEXT);

    const CouplingMap local_structure = CouplingMapGenerator::BuildInput(
      root_dir / options_.STRUCTURE_FILE,
      logger_.get(),
      "Merge mps");

    // TODO Investigate why following check
    // TODO creates a segfault when structure.txt is empty
    // if (local_structure.empty())
    // {
    //     logger_->display_message("Nothing to merge. Returning empty problem.",
    //     LogUtils::LOGLEVEL::INFO, MERGE_MPS_LOGGER_CONTEXT); return;
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

    logger_->display_message("Solving...", LogUtils::LOGLEVEL::INFO, MERGE_MPS_LOGGER_CONTEXT);

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
                                 MERGE_MPS_LOGGER_CONTEXT);
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
                               + " coupling constraints",
                             LogUtils::LOGLEVEL::INFO,
                             MERGE_MPS_LOGGER_CONTEXT);

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
                                   + " coupling constraints built",
                                 LogUtils::LOGLEVEL::INFO,
                                 MERGE_MPS_LOGGER_CONTEXT);
    }
    mstart.push_back(nb_elem);

    std::vector<double> rhs(nb_rows, 0);    // Constraints' rhs
    std::vector<char> qrtype(nb_rows, 'E'); // Constraints' types

    solver_addrows(*ptr_merged_solver_, qrtype, rhs, {}, mstart, mclind, dmatval);
}
