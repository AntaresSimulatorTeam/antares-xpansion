#include "SubproblemWorker.h"

#include "solver_utils.h"

/*!
 *  \brief Constructor of a Worker Slave
 *
 *  \param variable_map : Map of linking each variable of the problem to its id
 *
 *  \param problem_name : Name of the problem
 *
 */
SubproblemWorker::SubproblemWorker(
    VariableMap const &variable_map, const std::filesystem::path &path_to_mps,
    double const &slave_weight, const std::string &solver_name,
    const int log_level, SolverLogManager&solver_log_manager,
    Logger logger)
    : Worker(logger) {
  init(variable_map, path_to_mps, solver_name, log_level, solver_log_manager);

  int mps_ncols(_solver->get_ncols());
  DblVector obj_func_coeffs(mps_ncols);
  IntVector sequence(mps_ncols);
  for (int i = 0; i < mps_ncols; ++i) {
    sequence[i] = i;
  }
  solver_get_obj_func_coeffs(*_solver, obj_func_coeffs, 0, mps_ncols - 1);
  for (auto &c : obj_func_coeffs) {
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
void SubproblemWorker::fix_to(Point const &x0) const {
  int nbnds((int)_name_to_id.size());
  std::vector<int> indexes(nbnds);
  std::vector<char> bndtypes(nbnds, 'B');
  std::vector<double> values(nbnds);

  int i(0);
  for (auto const &kvp : _id_to_name) {
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
void SubproblemWorker::get_subgradient(Point &s) const {
  s.clear();
  std::vector<double> ptr(_solver->get_ncols());
  solver_getlpreducedcost(_solver, ptr);
  for (auto const &kvp : _id_to_name) {
    s[kvp.second] = +ptr[kvp.first];
  }
}

/*!
 *  \brief Return the solutions values of a problem
 *
 *  \param lb : reference to a map
 */
void SubproblemWorker::get_solution(PlainData::Variables &vars) const {
  vars.values = std::vector<double>(_solver->get_ncols());

  if (_solver->get_n_integer_vars() > 0) {
    _solver->get_mip_sol(vars.values.data());
  } else {
    _solver->get_lp_sol(vars.values.data(), NULL, NULL);
  }

  vars.names = _solver->get_col_names();
}