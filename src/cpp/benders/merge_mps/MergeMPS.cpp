#include "antares-xpansion/benders/merge_mps/MergeMPS.h"

#include <filesystem>
#include <numeric>
#include <utility>

#include "antares-xpansion/benders/benders_core/CouplingMapGenerator.h"
#include "antares-xpansion/benders/merge_mps/StandardLp.h"
#include "antares-xpansion/helpers/Timer.h"

MergeMPS::MergeMPS(MergeMPSOptions options,
                   Logger logger,
                   std::shared_ptr<Output::OutputWriter> writer):
    _options(std::move(options)),
    _logger(std::move(logger)),
    _writer(std::move(writer))
{
}

/**
 * Limitation: on windows may not support master problem with full path as name
 */
void MergeMPS::launch()
{
    const auto input_root_dir = std::filesystem::path(_options.INPUTROOT);
    const auto structure_path(input_root_dir / _options.STRUCTURE_FILE);

    const CouplingMap input = CouplingMapGenerator::BuildInput(structure_path,
                                                               _logger.get(),
                                                               "Merge mps");

    SolverFactory factory;
    const std::string solver_to_use = (_options.SOLVER_NAME == "COIN") ? "CBC"
                                                                       : _options.SOLVER_NAME;
    SolverAbstract::Ptr ptr_merged_solver = factory.create_solver(solver_to_use);

    ptr_merged_solver->set_output_log_level(_options.LOG_LEVEL);

    const CouplingMap candidates = get_candidates(input_root_dir,
                                                  input,
                                                  solver_to_use,
                                                  ptr_merged_solver);

    add_coupling_constraints(*ptr_merged_solver, candidates);

    _logger->display_message("Problems merged.");
    _logger->display_message("Writing mps file");
    ptr_merged_solver->write_prob_mps(std::filesystem::path(_options.OUTPUTROOT)
                                      / ("log_merged" + MPS_SUFFIX));
    _logger->display_message("Writing lp file");
    ptr_merged_solver->write_prob_lp(std::filesystem::path(_options.OUTPUTROOT) / "log_merged.lp");

    const bool is_optimal = solve(*ptr_merged_solver, 16);

    output_solution(*ptr_merged_solver, input, candidates, is_optimal);
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
 * \param ptr_merged_solver : Pointer to merged solver
 */
CouplingMap MergeMPS::get_candidates(const std::filesystem::path& root_dir,
                                     const CouplingMap& structure,
                                     const std::string& solver_to_use,
                                     SolverAbstract::Ptr& ptr_merged_solver)
{
    CouplingMap candidates;

    const int nb_sub_problems = structure.size() - 1;
    SolverFactory factory;

    _logger->display_message("Merging problems...");

    int current_prob_id{0};
    for (const auto& [filename, var_map]: structure)
    {
        SolverAbstract::Ptr ptr_solver = factory.create_solver(solver_to_use);

        ptr_solver->set_output_log_level(_options.LOG_LEVEL);

        ptr_solver->read_prob_mps(root_dir / filename);

        // Separate Master and Subproblems by a specific name ID
        // given at the beginning of the program
        if (filename != _options.MASTER_NAME)
        {
            const int nb_cols{ptr_solver->get_ncols()};

            IntVector indices(nb_cols);
            std::iota(indices.begin(), indices.end(), 0);

            DblVector obj_coeff(nb_cols);
            solver_get_obj_func_coeffs(*ptr_solver, obj_coeff, 0, nb_cols - 1);

            // Change the weight of coeff in the objective function
            // according to an options' rule
            const double weight = get_subproblem_weight(nb_sub_problems, filename);
            std::for_each(obj_coeff.begin(),
                          obj_coeff.end(),
                          [weight](double& coeff) { return coeff *= weight; });

            ptr_solver->chg_obj(indices, obj_coeff);
        }

        StandardLp lpData(*ptr_solver);
        const std::string var_prefix = "prob" + std::to_string(current_prob_id++) + "_";

        // Prefix the name of the problem (Master and slaves alike)
        // along with the counting
        lpData.append_in(ptr_merged_solver, var_prefix);

        for (const auto& [var_name, _]: var_map)
        {
            const int col_index = ptr_merged_solver->get_col_index(var_prefix + var_name);
            if (col_index == -1)
            {
                std::cerr << LOGLOCATION << "missing variable " << var_name << " in " << filename
                          << " supposedly renamed to " << var_prefix + var_name << ".";
                ptr_merged_solver->write_prob_lp(std::filesystem::path(_options.OUTPUTROOT)
                                             / "mergeError.lp");
                ptr_merged_solver->write_prob_mps(std::filesystem::path(_options.OUTPUTROOT)
                                              / ("mergeError" + MPS_SUFFIX));
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

/*!
 *  \brief Return subproblem weight value
 *
 *  \param nb_subproblems : total number of subproblems
 *
 *  \param name : subproblem name
 */
double MergeMPS::get_subproblem_weight(const int nb_subproblems, const std::string& name) const
{
    if (_options.SLAVE_WEIGHT == SUBPROBLEM_WEIGHT_UNIFORM_CST_STR)
    {
        return 1.0 / nb_subproblems;
    }
    else if (_options.SLAVE_WEIGHT == SUBPROBLEM_WEIGHT_CST_STR)
    {
        return 1.0 / _options.SLAVE_WEIGHT_VALUE;
    }
    else
    {
        const auto found = _options.weights.find(name);
        if (found == _options.weights.end())
        {
            _logger->display_message("No weight found for " + name
                                       + ". Problem will not contribute to objective function",
                                     LogUtils::LOGLEVEL::WARNING,
                                     "MergeMPS");
            // return 0.; //TODO Return 0 in this case
        }
        return found->second;
    }
}

/**
 * \brief Add coupling equality constraints between subproblems
 *
 * \param merged_solver : Ref to merged solver
 *
 * \param candidates : Mapping of investment candidates
 */
void MergeMPS::add_coupling_constraints(SolverAbstract& merged_solver,
                                        const CouplingMap& candidates)
{
    size_t nb_elem_reserve{0}; // 2-permutation of variables

    for (const auto& [_, file_mapping]: candidates)
    {
        nb_elem_reserve += file_mapping.size() * (file_mapping.size() - 1);
    }

    const size_t nb_rows_reserve = nb_elem_reserve / 2; // choose-2-combination of variables

    _logger->display_message("About to add " + std::to_string(nb_rows_reserve)
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
        _logger->display_message(var_name);

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
    solver_addrows(merged_solver, constraint_type, rhs, {}, var_offsets, var_indices, var_values);
}

/**
 * \brief Solve merged problem
 *
 * \param merged_solver : Ref to merged solver
 *
 * \param nb_threads : Number of threads to use
 */
bool MergeMPS::solve(SolverAbstract& merged_solver, const int nb_threads)
{
    merged_solver.set_threads(nb_threads);

    _logger->display_message("Solving...");

    Timer timer;
    int status{0};

    if (merged_solver.get_n_integer_vars() > 0)
    {
        status = merged_solver.solve_mip();
    }
    else
    {
        status = merged_solver.solve_lp();
    }

    _logger->log_total_duration(timer.elapsed());

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
void MergeMPS::output_solution(SolverAbstract& merged_solver,
                               const CouplingMap& structure,
                               const CouplingMap& candidates,
                               const bool is_sol_optimal)
{
    double overall_cost{0}, investment_cost{0}, operational_cost{0};
    DblVector solution(merged_solver.get_ncols()), obj_coeff(merged_solver.get_ncols());

    if (merged_solver.get_n_integer_vars() > 0)
    {
        overall_cost = merged_solver.get_mip_value();
        merged_solver.get_mip_sol(solution.data());
    }
    else
    {
        overall_cost = merged_solver.get_lp_value();
        merged_solver.get_lp_sol(solution.data(), nullptr, nullptr);
    }

    merged_solver.get_obj(obj_coeff.data(), 0, merged_solver.get_ncols() - 1);

    std::map<std::string, double> investments;
    for (const auto& [var_name, _]: structure.at(_options.MASTER_NAME))
    {
        const int var_idx_in_merged = candidates.at(var_name).at(_options.MASTER_NAME);
        investments[var_name] = solution[var_idx_in_merged];
        investment_cost += investments[var_name] * obj_coeff[var_idx_in_merged];
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

    _writer->update_solution(sol_infos);
    _writer->dump();
}
