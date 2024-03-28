#pragma once
#include "SubproblemCut.h"

class OuterLoopBiLevel {
 public:
  explicit OuterLoopBiLevel(const ExternalLoopOptions &options);
  bool Update_bilevel_data_if_feasible(
      const SubProblemDataMap &x,
      const std::vector<double> &outer_loop_criterion, double overall_cost,
      double invest_cost_at_x, double lambda_min);
  bool Check_bilevel_feasibility(
      const std::vector<double> &outer_loop_criterion, double overall_cost);
  //   double LambdaMax() const { return lambda_max_; }
  void SetOptions(const ExternalLoopOptions &options) { options_ = options; }
  void Init(const std::vector<double> &obj, const Point &max_invest,
            const VariableMap &master_variable);
  double LambdaMax() const { return lambda_max_; }
  double LambdaMin() const { return lambda_min_; }
  void SetLambda(double lambda) { lambda_ = lambda; }
  double BilevelBestub() const { return bilevel_best_ub_; }
  bool FoundFeasible() const { return found_feasible_; }

 private:
  void SetLambdaMaxToMaxInvestmentCosts(const std::vector<double> &obj,
                                        const Point &max_invest,
                                        const VariableMap &master_variable);
  void Update(const SubProblemDataMap &x, double overall_cost,
              double invest_cost_at_x);
  bool IsCriterionSatisfied(const std::vector<double> &outer_loop_criterions);
  bool found_feasible_ = false;
  double bilevel_best_ub_ = +1e20;
  SubProblemDataMap bilevel_best_x_;
  double lambda_max_ = 0.0;
  double lambda_min_ = 0.0;
  double lambda_ = 0.0;
  ExternalLoopOptions options_;
  std::vector<double> EXT_LOOP_CRITERION_VALUES_;
};