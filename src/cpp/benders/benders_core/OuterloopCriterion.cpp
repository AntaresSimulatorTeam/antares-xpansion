#pragma once
#include "OuterLoopCriterion.h"

OuterloopCriterionLOL::OuterloopCriterionLOL(double threshold, double epsilon)
    : threshold_(threshold), epsilon_(epsilon) {}

CRITERION OuterloopCriterionLOL::IsCriterionSatisfied(
    const WorkerMasterData& worker_master_data) {
  double sum_loss = ProcessSum(worker_master_data);
  CRITERION ret = (sum_loss >= threshold_ - epsilon_)
                      ? (sum_loss <= threshold_ + epsilon_) ? CRITERION::EQUAL
                                                            : CRITERION::GREATER
                      : CRITERION::LESSER;
  // CRITERION ret = (sum_loss <= threshold_ - epsilon_)   ? CRITERION::LESSER
  //                 : (sum_loss >= threshold_ + epsilon_) ? CRITERION::GREATER
  //                                                       : CRITERION::EQUAL;
  return ret;
}

double OuterloopCriterionLOL::ProcessSum(
    const WorkerMasterData& worker_master_data) {
  double sum_loss = 0;
  for (const auto& [sub_problem_name, sub_problem_data] :
       worker_master_data._cut_trace) {
    for (auto i(0); i < sub_problem_data.variables.names.size(); ++i) {
      auto var_name = sub_problem_data.variables.names[i];
      auto solution = sub_problem_data.variables.values[i];
      if (std::regex_search(var_name, rgx_) &&
          solution > UNSUPPLIED_ENERGY_MAX) {
        // 1h of unsupplied energy
        sum_loss += 1;
      }
    }
  }

  return sum_loss;
}
