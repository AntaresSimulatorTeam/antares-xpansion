
#include "CapexAnalysis.h"

#include <ProblemModifierCapex.h>
#include <SensitivityStudy.h>

#include <utility>
CapexAnalysis::CapexAnalysis(
    SensitivityInputData input_data,
    std::shared_ptr<SensitivityILogger> logger)
    : logger(std::move(logger)),
      input_data(std::move(input_data)){

}

std::pair<SinglePbData, SinglePbData>  CapexAnalysis::run() {
  ProblemModifierCapex pb_modifier(input_data.epsilon, input_data.best_ub, input_data.last_master);
  unsigned int nb_candidates = input_data.name_to_id.size();
  sensitivity_pb_model = pb_modifier.changeProblem(nb_candidates);
  auto min_capex_data = run_optimization(SensitivityStudy::StudyType::MINIMIZE);
  auto max_capex_data = run_optimization(SensitivityStudy::StudyType::MAXIMIZE);
  return { min_capex_data, max_capex_data };
}

SinglePbData CapexAnalysis::run_optimization(
    SensitivityStudy::StudyType minimize) {
  sensitivity_pb_model->chg_obj_direction(static_cast<bool>(minimize));

  SinglePbData pb_data = SinglePbData(SensitivityPbType::CAPEX, "capex", "",
                                      minimize == SensitivityStudy::StudyType::MINIMIZE ? MIN_C : MAX_C);
  logger->log_begin_pb_resolution(pb_data);

  auto raw_output = solve_sensitivity_pb();

  fill_single_pb_data(pb_data, raw_output);
  logger->log_pb_solution(pb_data);

  return pb_data;
}

RawPbData CapexAnalysis::solve_sensitivity_pb() const {
  RawPbData raw_output;
  int ncols = sensitivity_pb_model->get_ncols();

  raw_output.obj_coeffs.resize(ncols);
  raw_output.solution.resize(ncols);

  sensitivity_pb_model->get_obj(raw_output.obj_coeffs.data(), 0, ncols - 1);

  if (sensitivity_pb_model->get_n_integer_vars() > 0) {
    raw_output.status = sensitivity_pb_model->solve_mip();
    sensitivity_pb_model->get_mip_sol(raw_output.solution.data());
    raw_output.obj_value = sensitivity_pb_model->get_mip_value();
  } else {
    raw_output.status = sensitivity_pb_model->solve_lp();
    sensitivity_pb_model->get_lp_sol(raw_output.solution.data(), nullptr,
                                    nullptr);
    raw_output.obj_value = sensitivity_pb_model->get_lp_value();
  }

  return raw_output;
}

void CapexAnalysis::fill_single_pb_data(
    SinglePbData &pb_data, const RawPbData &raw_output) const {
  pb_data.objective = raw_output.obj_value;
  pb_data.solver_status = raw_output.status;

  for (auto const &kvp : input_data.name_to_id) {
    pb_data.candidates[kvp.first] = raw_output.solution[kvp.second];
  }
  pb_data.system_cost = get_system_cost(raw_output);
}

double CapexAnalysis::get_system_cost(const RawPbData &raw_output) const {
  int ncols = input_data.last_master->get_ncols();
  std::vector<double> master_obj(ncols);

  input_data.last_master->get_obj(master_obj.data(), 0, ncols - 1);

  double system_cost = 0;
  for (int col_id(0); col_id < ncols; col_id++) {
    system_cost += master_obj[col_id] * raw_output.solution[col_id];
  }
  return system_cost;
}
