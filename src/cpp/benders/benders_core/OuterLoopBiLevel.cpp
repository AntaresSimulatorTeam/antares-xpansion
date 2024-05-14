#include "OuterLoopBiLevel.h"

OuterLoopBiLevel::OuterLoopBiLevel(const Outerloop::OuterLoopInputData  &outer_loop_input_data)
    : outer_loop_input_data_(outer_loop_input_data) {}

bool OuterLoopBiLevel::Update_bilevel_data_if_feasible(
    const Point &x, const std::vector<double> &outer_loop_criterion,
    double overall_cost, double invest_cost_at_x, double lambda_min) {
  if (found_feasible_ =
          Check_bilevel_feasibility(outer_loop_criterion, overall_cost);
      found_feasible_) {
    Update(x, overall_cost, invest_cost_at_x);
  } else {
    lambda_min_ = lambda_min;
  }

  return found_feasible_;
}

bool OuterLoopBiLevel::Check_bilevel_feasibility(
    const std::vector<double> &outer_loop_criterions, double overall_cost) {
  return IsCriterionSatisfied(outer_loop_criterions) &&
         overall_cost < bilevel_best_ub_;

  /**
   * else if method == opt ...
   */
}

bool OuterLoopBiLevel::IsCriterionSatisfied(
    const std::vector<double> &outer_loop_criterions) {
 const auto& outer_loop_input_data = outer_loop_input_data_.OuterLoopData();
  for (int index(0); index < outer_loop_criterions.size(); ++index) {
    if (outer_loop_criterions[index] >
        outer_loop_input_data[index].Criterion() +
            outer_loop_input_data_.CriterionTolerance()) {
      return false;
    }
  }
  return true;
}

void OuterLoopBiLevel::Update(const Point &x, double overall_cost,
                              double invest_cost_at_x) {
  bilevel_best_x_ = x;
  bilevel_best_ub_ = overall_cost;
  lambda_max_ = std::min(lambda_max_, invest_cost_at_x);
}

void OuterLoopBiLevel::Init(const std::vector<double> &obj,
                            const Point &max_invest,
                            const VariableMap &master_variable) {
  // check lambda_max_
  if (lambda_max_ <= 0 || lambda_max_ < lambda_min_) {
    // TODO log
    SetLambdaMaxToMaxInvestmentCosts(obj, max_invest, master_variable);
  }
  found_feasible_ = false;
}
void OuterLoopBiLevel::SetLambdaMaxToMaxInvestmentCosts(
    const std::vector<double> &obj, const Point &max_invest,
    const VariableMap &master_variable) {
  lambda_max_ = 0;
  for (const auto &[var_name, var_id] : master_variable) {
    lambda_max_ += obj[var_id] * max_invest.at(var_name);
  }
}
