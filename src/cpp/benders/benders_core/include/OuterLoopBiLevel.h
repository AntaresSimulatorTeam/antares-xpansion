#pragma once
#include "OuterLoopInputDataReader.h"
#include "SubproblemCut.h"

class OuterLoopBiLevel {
 public:
  explicit OuterLoopBiLevel(const Outerloop::OuterLoopInputData  &outer_loop_input_data);
  OuterLoopBiLevel() = default;
  bool Update_bilevel_data_if_feasible(
      const Point &x,
      const std::vector<double> &outer_loop_criterion, double overall_cost,
      double invest_cost_at_x, double lambda_min);
  bool Check_bilevel_feasibility(
      const std::vector<double> &outer_loop_criterion, double overall_cost);
  //   double LambdaMax() const { return lambda_max_; }
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
  void Update(const Point &x, double overall_cost,
              double invest_cost_at_x);
  bool IsCriterionSatisfied(const std::vector<double> &outer_loop_criterions);
  bool found_feasible_ = false;
  double bilevel_best_ub_ = +1e20;
  Point bilevel_best_x_;
  double lambda_max_ = 0.0;
  double lambda_min_ = 0.0;
  double lambda_ = 0.0;
  Outerloop::OuterLoopInputData outer_loop_input_data_;
};