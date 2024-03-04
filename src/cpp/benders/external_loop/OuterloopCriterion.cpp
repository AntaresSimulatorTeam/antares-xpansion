#pragma once
#include "OuterLoopCriterion.h"

#include "LoggerUtils.h"

OuterloopCriterionLossOfLoad::OuterloopCriterionLossOfLoad(
    const ExternalLoopOptions& options)
    : options_(options) {}

CRITERION OuterloopCriterionLossOfLoad::IsCriterionSatisfied(
    const WorkerMasterData& worker_master_data) {
  ProcessSum(worker_master_data);

  if (sum_loss_ <= options_.EXT_LOOP_CRITERION_VALUE +
                       options_.EXT_LOOP_CRITERION_TOLERANCE) {
    if (sum_loss_ >= options_.EXT_LOOP_CRITERION_VALUE -
                         options_.EXT_LOOP_CRITERION_TOLERANCE) {
      return CRITERION::IS_MET;
    }
    return CRITERION::LOW;
  } else {
    return CRITERION::HIGH;
  }
}

void OuterloopCriterionLossOfLoad::ProcessSum(
    const WorkerMasterData& worker_master_data) {
  sum_loss_ = 0;
  for (const auto& [sub_problem_name, sub_problem_data] :
       worker_master_data._cut_trace) {
    for (auto i(0); i < sub_problem_data.variables.names.size(); ++i) {
      auto var_name = sub_problem_data.variables.names[i];
      auto solution = sub_problem_data.variables.values[i];
      if (std::regex_search(var_name, rgx_) &&
          solution > options_.EXT_LOOP_CRITERION_COUNT_THRESHOLD) {
        // 1h of unsupplied energy
        sum_loss_ += 1;
      }
    }
  }
}

std::string OuterloopCriterionLossOfLoad::StateAsString() const {
  std::ostringstream msg;
  msg << "Sum loss = " << sum_loss_ << "\n"
      << "threshold: " << options_.EXT_LOOP_CRITERION_VALUE << "\n"
      << "epsilon: " << options_.EXT_LOOP_CRITERION_TOLERANCE << "\n";

  return msg.str();
}
