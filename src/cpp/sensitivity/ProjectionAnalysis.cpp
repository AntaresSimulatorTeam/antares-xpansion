//
// Created by Archfiend on 14/03/2022.
//

#include "ProjectionAnalysis.h"

#include <utility>

#include "AnalysisFunctions.h"
#include "ProblemModifierProjection.h"
#include "SensitivityStudy.h"
ProjectionAnalysis::ProjectionAnalysis(
    SensitivityInputData input_data, std::string candidate_name,
    std::shared_ptr<SensitivityILogger> logger)
    : Analysis(input_data, candidate_name, logger,
               SensitivityPbType::PROJECTION) {}

std::pair<SinglePbData, SinglePbData> ProjectionAnalysis::run() {
  const auto input_data_ = get_input_data();
  const auto candidate_name_ = get_candidate_name();
  ProblemModifierProjection problemModifierProjection(
      input_data_.epsilon, input_data_.best_ub, input_data_.last_master,
      input_data_.name_to_id.at(candidate_name_), candidate_name_);
  set_sensitivity_pb_model(
      problemModifierProjection.changeProblem(input_data_.name_to_id.size()));
  auto min_projection_data =
      run_optimization(SensitivityStudy::StudyType::MINIMIZE);
  auto max_projection_data =
      run_optimization(SensitivityStudy::StudyType::MAXIMIZE);
  return {min_projection_data, max_projection_data};
}
