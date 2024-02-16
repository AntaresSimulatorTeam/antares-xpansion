#pragma once
#include "OuterLoopCriterion.h"

#include "LoggerUtils.h"

OuterloopCriterionLOL::OuterloopCriterionLOL(double threshold, double epsilon)
    : threshold_(threshold), epsilon_(epsilon) {}

CRITERION OuterloopCriterionLOL::IsCriterionSatisfied(
    const WorkerMasterData& worker_master_data) {
  ProcessSum(worker_master_data);

  if (sum_loss_ <= threshold_ + epsilon_) {
    if (sum_loss_ >= threshold_ - epsilon_) {
      return CRITERION::IS_MET;
    }
    return CRITERION::LOW;
  } else {
    return CRITERION::HIGH;
  }
}

void OuterloopCriterionLOL::ProcessSum(
    const WorkerMasterData& worker_master_data) {
  sum_loss_ = 0;
  for (const auto& [sub_problem_name, sub_problem_data] :
       worker_master_data._cut_trace) {
    for (auto i(0); i < sub_problem_data.variables.names.size(); ++i) {
      auto var_name = sub_problem_data.variables.names[i];
      auto solution = sub_problem_data.variables.values[i];
      if (std::regex_search(var_name, rgx_) &&
          solution > UNSUPPLIED_ENERGY_MAX) {
        // 1h of unsupplied energy
        sum_loss_ += 1;
      }
    }
  }
}

std::string OuterloopCriterionLOL::StateAsString() const {
  std::ostringstream msg;
  msg << "Sum loss = " << sum_loss_ << "\n"
      << "threshold: " << threshold_ << "\n"
      << "epsilon: " << epsilon_ << "\n";

  return msg.str();
}
