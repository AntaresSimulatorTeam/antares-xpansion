#include "antares-xpansion/benders/merge_mps/MergeMPS.h"

#include <filesystem>
#include <numeric>
#include <ranges>
#include <utility>

#include "antares-xpansion/benders/benders_core/CouplingMapGenerator.h"
#include "antares-xpansion/benders/merge_mps/StandardLp.h"
#include "antares-xpansion/helpers/Timer.h"

AbstractMergeMPS::AbstractMergeMPS(MergeMPSOptions options,
                                   Logger logger,
                                   std::shared_ptr<Output::OutputWriter> writer):
    _options(std::move(options)),
    _logger(std::move(logger)),
    _writer(std::move(writer))
{
    if (_options.SOLVER_NAME == "COIN")
    {
        _options.SOLVER_NAME = "CBC";
    }

    SolverFactory factory;
    _ptr_merged_solver = factory.create_solver(_options.SOLVER_NAME);

    _ptr_merged_solver->set_output_log_level(_options.LOG_LEVEL);
}

/**
 * Limitation: on windows may not support master problem with full path as name
 */
void AbstractMergeMPS::launch()
{
    build_problem();

    export_problem();

    const bool is_optimal = solve();

    output_solution(is_optimal);
}

/**
 * \brief Build merged problem
 */
void AbstractMergeMPS::build_problem()
{
    const auto input_root_dir = std::filesystem::path(_options.INPUTROOT);
    const auto structure_path(input_root_dir / _options.STRUCTURE_FILE);

    _structure = CouplingMapGenerator::BuildInput(structure_path, _logger.get(), "Merge mps");

    // TODO Investigate why following check
    // TODO creates a segfault when structure.txt is empty
    // if (_structure.empty())
    // {
    //     _logger->display_message("Nothing to merge. Returning empty problem.");
    //     return;
    // }

    const int nb_sub_problems = _structure.size() - 1;
    const auto root_dir = std::filesystem::path(_options.INPUTROOT);
    SolverFactory factory;

    _logger->display_message("Merging problems...");

    int current_prob_id{0};
    for (auto& [filename, var_map]: _structure)
    {
        SolverAbstract::Ptr ptr_solver = factory.create_solver(_options.SOLVER_NAME);

        ptr_solver->set_output_log_level(_options.LOG_LEVEL);

        ptr_solver->read_prob_mps(root_dir / filename);

        // Separate Master and Subproblems by a specific name ID
        // given in the options file
        if (filename != _options.MASTER_NAME)
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
        lpData.append_in(*_ptr_merged_solver, var_prefix);

        for (auto& [var_name, var_idx]: var_map)
        {
            const int merged_col_index = _ptr_merged_solver->get_col_index(var_prefix + var_name);
            if (merged_col_index == -1)
            {
                const auto output_root = std::filesystem::path(_options.OUTPUTROOT);
                std::cerr << LOGLOCATION << "missing variable " << var_name << " in " << filename
                          << " supposedly renamed to " << var_prefix + var_name << ".";
                _ptr_merged_solver->write_prob_lp(output_root / "mergeError.lp");
                _ptr_merged_solver->write_prob_mps(output_root / ("mergeError" + MPS_SUFFIX));
                std::exit(1);
            }
            // TODO Not yet happy with this part
            // TODO Think of a better data struct maybe?
            var_idx = merged_col_index;
        }
    }

    add_coupling_constraints();
}

/**
 * \brief Export problem into mps and lp files
 */
void AbstractMergeMPS::export_problem()
{
    const auto output_root = std::filesystem::path(_options.OUTPUTROOT);
    _logger->display_message("Problems merged.");
    _logger->display_message("Writing mps file");
    _ptr_merged_solver->write_prob_mps(output_root / ("log_merged" + MPS_SUFFIX));
    _logger->display_message("Writing lp file");
    _ptr_merged_solver->write_prob_lp(output_root / "log_merged.lp");
}

/**
 * \brief Solve merged problem
 *
 * \param merged_solver : Ref to merged solver
 *
 * \param nb_threads : Number of threads to use
 */
bool AbstractMergeMPS::solve(const int nb_threads)
{
    _ptr_merged_solver->set_threads(nb_threads);

    _logger->display_message("Solving...");

    Timer timer;
    int status{0};

    if (_ptr_merged_solver->get_n_integer_vars() > 0)
    {
        status = _ptr_merged_solver->solve_mip();
    }
    else
    {
        status = _ptr_merged_solver->solve_lp();
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
void AbstractMergeMPS::output_solution(const bool is_sol_optimal)
{
    double overall_cost{0}, investment_cost{0}, operational_cost{0};
    DblVector solution(_ptr_merged_solver->get_ncols()), obj_coeff(_ptr_merged_solver->get_ncols());

    if (_ptr_merged_solver->get_n_integer_vars() > 0)
    {
        overall_cost = _ptr_merged_solver->get_mip_value();
        _ptr_merged_solver->get_mip_sol(solution.data());
    }
    else
    {
        overall_cost = _ptr_merged_solver->get_lp_value();
        _ptr_merged_solver->get_lp_sol(solution.data(), nullptr, nullptr);
    }

    _ptr_merged_solver->get_obj(obj_coeff.data(), 0, _ptr_merged_solver->get_ncols() - 1);

    std::map<std::string, double> investments;
    for (const auto& [var_name, var_idx]: _structure[_options.MASTER_NAME])
    {
        investments[var_name] = solution[var_idx];
        investment_cost += investments[var_name] * obj_coeff[var_idx];
    }
    if (investments.empty())
    {
        std::cerr << LOGLOCATION << "Could not find '" << _options.MASTER_NAME
                  << "' in structure\n";
    }

    operational_cost = overall_cost - investment_cost;

    Output::SolutionData sol_infos;
    sol_infos.nbWeeks_p = static_cast<int>(_structure.size());

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

MergeMasterSubproblemMPS::MergeMasterSubproblemMPS(MergeMPSOptions options,
                                                   Logger logger,
                                                   std::shared_ptr<Output::OutputWriter> writer):
    AbstractMergeMPS(options, logger, writer)
{
}

/*!
 *  \brief Return subproblem weight value
 *
 *  \param nb_subproblems : total number of subproblems
 *
 *  \param name : subproblem name
 */
double MergeMasterSubproblemMPS::get_objective_weight(const int nb_subproblems,
                                                      const std::string& name) const
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
            return 0.;
        }
        return found->second;
    }
}

/**
 * \brief Add coupling equality constraints between subproblems
 */
void MergeMasterSubproblemMPS::add_coupling_constraints()
{
    std::map<std::string, std::vector<int>> variables;
    for (const auto& [_, var_map]: _structure)
    {
        for (const auto& [var_name, var_idx]: var_map)
        {
            variables[var_name].push_back(var_idx);
        }
    }

    // Add n-1 new constraints per variable where n is
    // the number of columns representing this variable
    const size_t nb_rows_reserve = std::accumulate(variables.cbegin(),
                                                   variables.cend(),
                                                   size_t{0},
                                                   [](size_t acc, const auto& pair)
                                                   {
                                                       const auto& indices = pair.second;
                                                       return acc + indices.size() - 1;
                                                   });
    const size_t nb_elem_reserve = 2 * nb_rows_reserve;

    _logger->display_message("About to add " + std::to_string(nb_rows_reserve)
                             + " coupling constraints");

    IntVector mstart;  // Constraints' offsets
    mstart.reserve(nb_rows_reserve + 1);

    IntVector mclind;  // Variables' indices
    DblVector dmatval; // Variables' values
    dmatval.reserve(nb_elem_reserve);
    mclind.reserve(nb_elem_reserve);

    int nb_rows{0};
    int nb_elem{0};
    for (const auto& [var_name, indices]: variables)
    {
        _logger->display_message(var_name + " : " + std::to_string(indices.size() - 1)
                                 + " coupling constraints added");

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
    }
    mstart.push_back(nb_elem);

    DblVector rhs(nb_rows, 0);       // Constraints' rhs
    CharVector qrtype(nb_rows, 'E'); // Constraints' types

    solver_addrows(*_ptr_merged_solver, qrtype, rhs, {}, mstart, mclind, dmatval);
}
