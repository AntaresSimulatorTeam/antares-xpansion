#include "AnalysisFunctions.h"
RawPbData solve_sensitivity_pb(const SensitivityInputData& input_data,
                               const SolverAbstract::Ptr& sensitivity_problem) {
  RawPbData raw_output;
  int ncols = sensitivity_problem->get_ncols();

  raw_output.obj_coeffs.resize(ncols);
  raw_output.solution.resize(ncols);

  sensitivity_problem->get_obj(raw_output.obj_coeffs.data(), 0, ncols - 1);

  std::ifstream basis_file(input_data.basis_file_path);
  if (basis_file.good()) {
    sensitivity_problem->read_basis(input_data.basis_file_path);
  }

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

SolverAbstract::Ptr get_sensitivity_problem(
    const SensitivityInputData input_data, const std::string& candidate_name,
    SensitivityPbType type) {
  unsigned int nb_candidates = input_data.name_to_id.size();
  switch (type) {
    case SensitivityPbType::CAPEX: {
      ProblemModifierCapex pb_modifier(input_data.epsilon, input_data.best_ub,
                                       input_data.last_master);
      return pb_modifier.changeProblem(nb_candidates);
    }
    case SensitivityPbType::PROJECTION: {
      ProblemModifierProjection pb_modifier(
          input_data.epsilon, input_data.best_ub, input_data.last_master,
          input_data.name_to_id.at(candidate_name), candidate_name);
      return pb_modifier.changeProblem(nb_candidates);
    }

    default:
      std::cerr << "Unrecognized Sensitivity Problem type" << std::endl;
      return nullptr;
  }
}