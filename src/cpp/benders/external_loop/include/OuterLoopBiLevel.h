#pragma once
#include "SubproblemCut.h"

class OuterLoopBiLevel {
 public:
  OuterLoopBiLevel() = default;
  bool Update_bilevel_data_if_feasible(
      const SubProblemDataMap &x, double invest_cost_at_x,
      const std::vector<double> &outer_loop_criterion, double overall_cost);
  bool Check_bilevel_feasibility(
      const std::vector<double> &outer_loop_criterion, double overall_cost);
  double LambdaMax() const { return lambda_max_; }
  void SetOptions(const ExternalLoopOptions &options) { options_ = options; }

 private:
  void Update(const SubProblemDataMap &x, double invest_cost_at_x,
              double overall_cost);
  bool IsCriterionSatisfied(const std::vector<double> &outer_loop_criterions);
  bool found_feasible_ = false;
  double bilevel_best_ub_ = 1e20;
  SubProblemDataMap bilevel_best_x_;
  double lambda_max_ = 0.0;
  ExternalLoopOptions options_;
  std::vector<double> EXT_LOOP_CRITERION_VALUES_;
};