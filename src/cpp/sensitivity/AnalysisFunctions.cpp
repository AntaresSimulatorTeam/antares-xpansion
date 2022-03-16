#include "AnalysisFunctions.h"
RawPbData solve_sensitivity_pb(const SolverAbstract::Ptr &sensitivity_problem) {
  RawPbData raw_output;
  int ncols = sensitivity_problem->get_ncols();

  raw_output.obj_coeffs.resize(ncols);
  raw_output.solution.resize(ncols);

  sensitivity_problem->get_obj(raw_output.obj_coeffs.data(), 0, ncols - 1);

  if (sensitivity_problem->get_n_integer_vars() > 0) {
    raw_output.status = sensitivity_problem->solve_mip();
    sensitivity_problem->get_mip_sol(raw_output.solution.data());
    raw_output.obj_value = sensitivity_problem->get_mip_value();
  } else {
    raw_output.status = sensitivity_problem->solve_lp();
    sensitivity_problem->get_lp_sol(raw_output.solution.data(), nullptr,
                                    nullptr);
    raw_output.obj_value = sensitivity_problem->get_lp_value();
  }

  return raw_output;
}