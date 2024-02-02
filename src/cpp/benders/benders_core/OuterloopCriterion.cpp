#pragma once
#include "OuterLoopCriterion.h"

OuterloopCriterionLOL::OuterloopCriterionLOL(double threshold, double epsilon)
    : threshold_(threshold), epsilon_(epsilon) {}

bool OuterloopCriterionLOL::IsCriterionSatisfied(
    const BendersCuts& benders_cuts) {
  double sum_loss = ProcessSum(benders_cuts);

  return (threshold_ - epsilon_ <= sum_loss) &&
         (sum_loss <= threshold_ + epsilon_);
}

bool OuterloopCriterionLOL::ProcessSum(const BendersCuts& benders_cuts) {
  double sum_loss = 0;
  for (const auto& [sub_problem_name, sub_problem_data] :
       benders_cuts.subsProblemDataMap) {
    for (auto i(0); i < sub_problem_data.variables.names.size(); ++i) {
      auto var_name = sub_problem_data.variables.names[i];
      auto solution = sub_problem_data.variables.values[i];
      if (std::regex_search(var_name, rgx_) &&
          solution > UNSUPPLIED_ENERGY_MAX) {
        sum_loss += solution;
      }
    }
  }

  return sum_loss;
}
