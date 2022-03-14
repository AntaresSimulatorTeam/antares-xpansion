//
// Created by Archfiend on 14/03/2022.
//

#include "ProjectionAnalysis.h"
#include "ProblemModifierProjection.h"
#include "SensitivityStudy.h"

#include <utility>
ProjectionAnalysis::ProjectionAnalysis(
    SensitivityInputData input_data, std::string candidate_name,
    std::shared_ptr<SensitivityILogger> logger)
: logger(std::move(logger))
, input_data(std::move(input_data))
, candidate_name(std::move(candidate_name))
{

}

std::pair<SinglePbData, SinglePbData> ProjectionAnalysis::run() {
  ProblemModifierProjection problemModifierProjection(input_data.epsilon, input_data.best_ub, input_data.last_master, input_data.name_to_id.at(candidate_name), candidate_name);
  sensitivity_pb_model = problemModifierProjection.changeProblem(input_data.name_to_id.size());
  auto min_projection_data =
      run_optimization(SensitivityStudy::StudyType::MINIMIZE);
  auto max_projection_data =
      run_optimization(SensitivityStudy::StudyType::MAXIMIZE);
  return { min_projection_data, max_projection_data };
}

RawPbData solve_sensitivity_pb(
    const SolverAbstract::Ptr& sensitivity_problem) {
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

SinglePbData ProjectionAnalysis::run_optimization(
    SensitivityStudy::StudyType minimize) {
  sensitivity_pb_model->chg_obj_direction(static_cast<bool>(minimize));

  SinglePbData pb_data = {SensitivityPbType::PROJECTION, PROJECTION_C, candidate_name,
                          minimize == SensitivityStudy::StudyType::MINIMIZE ? MIN_C : MAX_C};
  logger->log_begin_pb_resolution(pb_data);

  auto raw_output = solve_sensitivity_pb(sensitivity_pb_model);

  fill_single_pb_data(pb_data, raw_output);
  logger->log_pb_solution(pb_data);

  return pb_data;
}

void ProjectionAnalysis::fill_single_pb_data(
    SinglePbData &pb_data, const RawPbData &raw_output) const {
  pb_data.objective = raw_output.obj_value;
  pb_data.solver_status = raw_output.status;

  for (auto const &kvp : input_data.name_to_id) {
    pb_data.candidates[kvp.first] = raw_output.solution[kvp.second];
  }
  pb_data.system_cost = get_system_cost(raw_output);
}

double ProjectionAnalysis::get_system_cost(const RawPbData &raw_output) const {
  int ncols = input_data.last_master->get_ncols();
  std::vector<double> master_obj(ncols);

  input_data.last_master->get_obj(master_obj.data(), 0, ncols - 1);

  double system_cost = 0;
  for (int col_id(0); col_id < ncols; col_id++) {
    system_cost += master_obj[col_id] * raw_output.solution[col_id];
  }
  return system_cost;
}