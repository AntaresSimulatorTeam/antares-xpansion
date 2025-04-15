#include "antares-xpansion/benders/merge_mps/MergeMPS.h"

#include <filesystem>
#include <numeric>
#include <utility>

#include "antares-xpansion/benders/benders_core/CouplingMapGenerator.h"
#include "antares-xpansion/benders/merge_mps/StandardLp.h"
#include "antares-xpansion/helpers/Timer.h"

AbstractMergeMPS::AbstractMergeMPS(MergeMPSOptions options,
                                   Logger logger,
                                   std::shared_ptr<Output::OutputWriter> writer):
    writer_(std::move(writer)),
    options_(std::move(options)),
    logger_(std::move(logger))
{
    if (options_.SOLVER_NAME == "COIN")
    {
        options_.SOLVER_NAME = "CBC";
    }

    SolverFactory factory;
    ptr_merged_solver_ = factory.create_solver(options_.SOLVER_NAME);

    ptr_merged_solver_->set_output_log_level(options_.LOG_LEVEL);
}

/**
 * Limitation: on windows may not support master problem with full path as name
 */
void AbstractMergeMPS::launch()
{
    const auto input_root_dir = std::filesystem::path(options_.INPUTROOT);
    const auto structure_path(input_root_dir / options_.STRUCTURE_FILE);

    const CouplingMap input = CouplingMapGenerator::BuildInput(structure_path,
                                                               logger_.get(),
                                                               "Merge mps");

    const CouplingMap candidates = get_candidates(input);

    add_coupling_constraints(candidates);

    export_problem();

    const bool is_optimal = solve();

    output_solution(input, candidates, is_optimal);
}

/**
 * \brief Return a mapping of investment candidates
 *
 * \param root_dir : Path to root directory where files are
 *
 * \param structure : Mapping of files and investments candidates
 *
 * \param solver_to_use : Solver name that will be used in back-end
 *
 * \param merged_solver : Ref to merged solver
 */
CouplingMap AbstractMergeMPS::get_candidates(const CouplingMap& structure)
{
    if (structure.empty())
    {
        return {};
    }

    CouplingMap candidates;

    const int nb_sub_problems = structure.size() - 1;
    const auto root_dir = std::filesystem::path(options_.INPUTROOT);
    SolverFactory factory;

    logger_->display_message("Merging problems...");

    int current_prob_id{0};
    for (const auto& [filename, var_map]: structure)
    {
        SolverAbstract::Ptr ptr_solver = factory.create_solver(options_.SOLVER_NAME);

        ptr_solver->set_output_log_level(options_.LOG_LEVEL);

        ptr_solver->read_prob_mps(root_dir / filename);

        // Separate Master and Subproblems by a specific name ID
        // given at the beginning of the program
        if (filename != options_.MASTER_NAME)
        {
            const int nb_cols{ptr_solver->get_ncols()};

            IntVector indices(nb_cols);
            std::iota(indices.begin(), indices.end(), 0);

            DblVector obj_coeff(nb_cols);
            solver_get_obj_func_coeffs(*ptr_solver, obj_coeff, 0, nb_cols - 1);

            // Change the weight of coeff in the objective function
            // according to an options' rule
            const double weight = get_objective_weight(nb_sub_problems, filename);
            std::for_each(obj_coeff.begin(),
                          obj_coeff.end(),
                          [weight](double& coeff) { return coeff *= weight; });

            ptr_solver->chg_obj(indices, obj_coeff);
        }

        StandardLp lpData(*ptr_solver);
        const std::string var_prefix = "prob" + std::to_string(current_prob_id++) + "_";

        // Prefix the name of the problem (Master and slaves alike)
        // along with the counting
        lpData.append_in(*ptr_merged_solver_, var_prefix);

        for (const auto& [var_name, _]: var_map)
        {
            const int col_index = ptr_merged_solver_->get_col_index(var_prefix + var_name);
            if (col_index == -1)
            {
                const auto output_root = std::filesystem::path(options_.OUTPUTROOT);
                std::cerr << LOGLOCATION << "missing variable " << var_name << " in " << filename
                          << " supposedly renamed to " << var_prefix + var_name << ".";
                ptr_merged_solver_->write_prob_lp(output_root / "mergeError.lp");
                ptr_merged_solver_->write_prob_mps(output_root / ("mergeError" + MPS_SUFFIX));
                std::exit(1);
            }
            else
            {
                // All this part serves to fill this map
                // that aggregates all variables into the
                // merged problem
                // [variable][subproblem] = col
                candidates[var_name][filename] = col_index;
            }
        }
    }

    return candidates;
}

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
 * \brief Solve merged problem
 *
 * \param merged_solver : Ref to merged solver
 *
 * \param nb_threads : Number of threads to use
 */
bool AbstractMergeMPS::solve(int nb_threads)
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
void AbstractMergeMPS::output_solution(const CouplingMap& structure,
                                       const CouplingMap& candidates,
                                       bool is_sol_optimal)
{
    double overall_cost{0}, investment_cost{0}, operational_cost{0};
    DblVector solution(ptr_merged_solver_->get_ncols()), obj_coeff(ptr_merged_solver_->get_ncols());

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

    std::map<std::string, double> investments;
    if (const auto master = structure.find(options_.MASTER_NAME); master != structure.end())
    {
        for (const auto& [var_name, _]: master->second)
        {
            const int var_idx_in_merged = candidates.at(var_name).at(options_.MASTER_NAME);
            investments[var_name] = solution[var_idx_in_merged];
            investment_cost += investments[var_name] * obj_coeff[var_idx_in_merged];
        }
    }
    else
    {
        std::cerr << LOGLOCATION << "Could not find '" << options_.MASTER_NAME
                  << "' in structure\n";
    }

    operational_cost = overall_cost - investment_cost;

    Output::SolutionData sol_infos;
    sol_infos.nbWeeks_p = static_cast<int>(structure.size());

    sol_infos.solution.lb = overall_cost;
    sol_infos.solution.ub = overall_cost;
    sol_infos.solution.investment_cost = investment_cost;
    sol_infos.solution.operational_cost = operational_cost;
    sol_infos.solution.overall_cost = overall_cost;

    sol_infos.solution.candidates.clear();
    sol_infos.solution.candidates.reserve(investments.size());

    for (const auto& [var_name, var_value]: investments)
    {
        const Output::CandidateData candidate_data{var_name, var_value, -1, -1};
        sol_infos.solution.candidates.push_back(candidate_data);
    }

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
double MergeMasterSubproblemMPS::get_objective_weight(int nb_subproblems,
                                                      const std::string& name) const
{
    if (options_.SLAVE_WEIGHT == SUBPROBLEM_WEIGHT_UNIFORM_CST_STR)
    {
        return 1.0 / nb_subproblems;
    }
    else if (options_.SLAVE_WEIGHT == SUBPROBLEM_WEIGHT_CST_STR)
    {
        return 1.0 / options_.SLAVE_WEIGHT_VALUE;
    }
    else
    {
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
}

/**
 * \brief Add coupling equality constraints between subproblems
 *
 * \param candidates : Mapping of investment candidates
 */
void MergeMasterSubproblemMPS::add_coupling_constraints(const CouplingMap& candidates)
{
    // TODO Investigate why following check
    // TODO creates a segfault when structure.txt is empty
    // if (candidates.empty())
    // {
    //     return;
    // }

    size_t nb_elem_reserve{0}; // 2-permutation of variables
    for (const auto& [_, file_mapping]: candidates)
    {
        nb_elem_reserve += file_mapping.size() * (file_mapping.size() - 1);
    }

    const size_t nb_rows_reserve = nb_elem_reserve / 2; // choose-2-combination of variables

    logger_->display_message("About to add " + std::to_string(nb_rows_reserve)
                             + " coupling constraints");

    IntVector var_offsets;
    IntVector var_indices;
    DblVector var_values;
    var_values.reserve(nb_elem_reserve);
    var_indices.reserve(nb_elem_reserve);
    var_offsets.reserve(nb_rows_reserve + 1);

    int nb_rows{0};
    int nb_elem{0};
    for (const auto& [var_name, file_map]: candidates)
    {
        logger_->display_message(var_name);

        auto it = file_map.cbegin();
        const int ref_var_idx = it->second;

        // Starting from second element onwards
        // Add one equality constraint per pair of variables:
        // 1 * ref - 1 * second = 0
        for (std::advance(it, 1); it != file_map.cend(); ++it)
        {
            const auto& [filename, var_idx] = *it;

            var_offsets.push_back(nb_elem);

            var_indices.push_back(ref_var_idx);
            var_values.push_back(1);
            ++nb_elem;

            var_indices.push_back(var_idx);
            var_values.push_back(-1);
            ++nb_elem;

            ++nb_rows;
        }
    }
    var_offsets.push_back(nb_elem);

    DblVector rhs(nb_rows, 0);
    CharVector constraint_type(nb_rows, 'E');

    solver_addrows(*ptr_merged_solver_,
                   constraint_type,
                   rhs,
                   {},
                   var_offsets,
                   var_indices,
                   var_values);
}
