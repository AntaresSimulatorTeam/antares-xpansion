
#include "CapexAnalysis.h"

#include <PbModifierCapex.h>
#include <SensitivityStudy.h>
CapexAnalysis::CapexAnalysis(
    const SensitivityInputData& input_data,
    std::shared_ptr<SensitivityILogger> logger)
    : logger(logger),
      input_data(input_data){

}
std::pair<SinglePbData, SinglePbData>  CapexAnalysis::run() {
  PbModifierCapex pb_modifier(input_data.epsilon, input_data.best_ub);
  int nb_candidates = input_data.name_to_id.size();
  auto sensitivity_pb_model =
      pb_modifier.changeProblem(nb_candidates, input_data.last_master);
  run_optimization(sensitivity_pb_model, SensitivityStudy::MINIMIZE);
  run_optimization(sensitivity_pb_model, SensitivityStudy::MAXIMIZE);
  return { SinglePbData(), SinglePbData() };
}

void CapexAnalysis::run_optimization(
    const SolverAbstract::Ptr &sensitivity_model, const bool minimize) {
  sensitivity_model->chg_obj_direction(minimize);

  SinglePbData pb_data = init_single_pb_data(minimize);
  logger->log_begin_pb_resolution(pb_data);

  //auto raw_output = solve_sensitivity_pb(sensitivity_model);

  //fill_single_pb_data(pb_data, raw_output);
  //logger->log_pb_solution(pb_data);

  //_output_data.pbs_data.push_back(pb_data);
}

SinglePbData CapexAnalysis::init_single_pb_data(
    const bool minimize) const {
  std::string candidate_name = "";
  std::string str_pb_type = "capex";
  std::string opt_dir = minimize ? MIN_C : MAX_C;
  return SinglePbData(SensitivityPbType::CAPEX, str_pb_type, candidate_name,
                      opt_dir);
}
