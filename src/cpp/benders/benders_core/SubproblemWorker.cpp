#include "antares-xpansion/benders/benders_core/SubproblemWorker.h"

#include <utility>

#include "antares-xpansion/helpers/solver_utils.h"

/*!
 *  \brief Constructor of a Worker Slave
 *
 *  \param variable_map : Map of linking each variable of the problem to its id
 *
 *  \param problem_name : Name of the problem
 *
 */
SubproblemWorker::SubproblemWorker(const VariableMap& variable_map,
                                   double slave_weight,
                                   const std::string& solver_name,
                                   int log_level,
                                   const SolverLogManager& solver_log_manager,
                                   Logger logger,
                                   ProblemsFormat format,
                                   IBendersProblemProvider* benders_problem_provider):
    Worker(variable_map, std::move(logger))
{
    init(solver_name, log_level, solver_log_manager, format, benders_problem_provider);
    int mps_ncols(_solver->get_ncols());
    DblVector obj_func_coeffs(mps_ncols);
    IntVector sequence(mps_ncols);
    for (int i = 0; i < mps_ncols; ++i)
    {
        sequence[i] = i;
    }
    solver_get_obj_func_coeffs(*_solver, obj_func_coeffs, 0, mps_ncols - 1);
    for (auto& c: obj_func_coeffs)
    {
        c *= slave_weight;
    }
    _solver->chg_obj(sequence, obj_func_coeffs);
}

SubproblemWorker::SubproblemWorker(VariableMap& variable_map,
                                   double slave_weight,              
                                   std::shared_ptr<SolverAbstract> solver,
                                   Logger logger
                                ):
    Worker(variable_map, std::move(logger)) 
{
    init_for_compact_in_mem(solver, variable_map);
    setup_obj(slave_weight);
}

void SubproblemWorker::setup_obj(double slave_weight)
{
    int mps_ncols(_solver->get_ncols());
    DblVector obj_func_coeffs(mps_ncols);
    IntVector sequence(mps_ncols);
    for (int i = 0; i < mps_ncols; ++i)
    {
        sequence[i] = i;
    }
    solver_get_obj_func_coeffs(*_solver, obj_func_coeffs, 0, mps_ncols - 1);
    for (auto& c: obj_func_coeffs)
    {
        c *= slave_weight;
    }
    _solver->chg_obj(sequence, obj_func_coeffs);
}

/*!
 *  \brief Fix a set of variables to constant in a problem
 *
 *  Method to set variables in a problem by fixing their bounds
 *
 *  \param x0 : Set of variables to fix
 */
void SubproblemWorker::fix_to(const Point& x0) const
{
    int nbnds((int)_name_to_id.size());
    std::vector<int> indexes(nbnds);
    std::vector<char> bndtypes(nbnds, 'B');
    std::vector<double> values(nbnds);

    int i(0);
    for (const auto& kvp: _id_to_name)
    {
        indexes[i] = kvp.first;
        values[i] = x0.find(kvp.second)->second;
        ++i;
    }

    solver_chgbounds(_solver, indexes, bndtypes, values);
}

/*!
 *  \brief Get LP solution value of a problem
 *
 *  \param s : Empty point which receives the solution
 */
void SubproblemWorker::get_subgradient(Point& subgradient) const
{
    subgradient.clear();
    std::vector<double> ptr(_solver->get_ncols());
    solver_getlpreducedcost(_solver, ptr);

    for (const auto& kvp: _id_to_name)
    {
        subgradient[kvp.second] = +ptr[kvp.first];
    }
}

/*!
 *  \brief Return the solutions values of a problem
 *
 *  \param lb : reference to a map
 */
std::vector<double> SubproblemWorker::get_solution() const
{
    std::vector<double> solution(_solver->get_ncols());

    if (_solver->get_n_integer_vars() > 0)
    {
        _solver->get_mip_sol(solution.data());
    }
    else
    {
        _solver->get_lp_sol(solution.data(), NULL, NULL);
    }
    return solution;
}

void SubproblemWorker::delete_rows(int start_pos)
{
    int num_rows = _solver->get_nrows();
    num_rows--;
    _solver->del_rows(start_pos, num_rows);
}

int SubproblemWorker::get_variable_index(const std::string& variable_name)
{
    int variable_index(-1);

    variable_index = _solver->get_col_index(variable_name);

    return variable_index;
}

int SubproblemWorker::get_problem_row_num()
{
    return _solver->get_nrows();
}
