#include "Analysis.h"

#include "AnalysisFunctions.h"
#include "SensitivityOutputData.h"
Analysis::Analysis(SensitivityInputData input_data, std::string candidate_name,
                   std::shared_ptr<SensitivityILogger> logger,
                   SensitivityPbType type)
    : input_data(std::move(input_data)),
      candidate_name(std::move(candidate_name)),
      logger(std::move(logger)),
      problem_type(std::move(type)) {}

void Analysis::fill_single_pb_data(SinglePbData &pb_data,
                                   const RawPbData &raw_output) const {
  pb_data.objective = raw_output.obj_value;
  pb_data.solver_status = raw_output.status;

  for (auto const &kvp : input_data.name_to_id) {
    pb_data.candidates[kvp.first] = raw_output.solution[kvp.second];
  }
  pb_data.system_cost = get_system_cost(raw_output);
}
double Analysis::get_system_cost(const RawPbData &raw_output) const {
  int ncols = input_data.last_master->get_ncols();
  std::vector<double> master_obj(ncols);

  input_data.last_master->get_obj(master_obj.data(), 0, ncols - 1);

  double system_cost = 0;
  for (int col_id(0); col_id < ncols; col_id++) {
    system_cost += master_obj[col_id] * raw_output.solution[col_id];
  }
  return system_cost;
}

SinglePbData Analysis::run_optimization(SensitivityStudy::StudyType minimize) {
  sensitivity_pb_model->chg_obj_direction(static_cast<bool>(minimize));

  SinglePbData pb_data = {
      problem_type, get_string_from_SensitivityPbType(problem_type),
      candidate_name,
      minimize == SensitivityStudy::StudyType::MINIMIZE ? MIN_C : MAX_C};
  logger->log_begin_pb_resolution(pb_data);

  auto raw_output = solve_sensitivity_pb(sensitivity_pb_model);

  fill_single_pb_data(pb_data, raw_output);
  logger->log_pb_solution(pb_data);

  return pb_data;
}
std::pair<SinglePbData, SinglePbData> Analysis::run() {
  sensitivity_pb_model =
      get_sensitivity_problem(input_data, candidate_name, problem_type);
  auto min_capex_data = run_optimization(SensitivityStudy::StudyType::MINIMIZE);
  auto max_capex_data = run_optimization(SensitivityStudy::StudyType::MAXIMIZE);
  return {min_capex_data, max_capex_data};
}