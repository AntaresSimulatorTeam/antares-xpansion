#include "AdequacyCriterionBiLevel.h"

AdequacyCriterionBiLevel::AdequacyCriterionBiLevel(
    const AdequacyCriterionSpace::AdequacyCriterionInputData
        &adequacy_criterion_input_data)
    : adequacy_criterion_input_data_(adequacy_criterion_input_data) {}

bool AdequacyCriterionBiLevel::Update_bilevel_data_if_feasible(
    const Point &x, const std::vector<double> &adequacy_criterion,
    double overall_cost, double invest_cost_at_x, double lambda) {
  if (found_feasible_ =
          Check_bilevel_feasibility(adequacy_criterion, overall_cost);
      found_feasible_) {
    Update(x, overall_cost, invest_cost_at_x);
  } else {
    lambda_min_ = lambda;
  }

  return found_feasible_;
}

bool AdequacyCriterionBiLevel::Check_bilevel_feasibility(
    const std::vector<double> &adequacy_criterions, double overall_cost) {
  return IsCriterionSatisfied(adequacy_criterions) &&
         overall_cost < bilevel_best_ub_;

  /**
   * else if method == opt ...
   */
}

bool AdequacyCriterionBiLevel::IsCriterionSatisfied(
    const std::vector<double> &adequacy_criterions) {
  const auto &adequacy_criterion_input_data =
      adequacy_criterion_input_data_.AdequacyCriterionData();
  for (int index(0); index < adequacy_criterions.size(); ++index) {
    if (adequacy_criterions[index] >
        adequacy_criterion_input_data[index].Criterion()) {
      return false;
    }
  }
  return true;
}

void AdequacyCriterionBiLevel::Update(const Point &x, double overall_cost,
                                      double invest_cost_at_x) {
  bilevel_best_x_ = x;
  bilevel_best_ub_ = overall_cost;
  lambda_max_ = std::min(lambda_max_, invest_cost_at_x);
}

void AdequacyCriterionBiLevel::Init(const std::vector<double> &obj,
                                    const Point &max_invest,
                                    const VariableMap &master_variable) {
  // TODO log
  SetLambdaMaxToMaxInvestmentCosts(obj, max_invest, master_variable);
  
  found_feasible_ = false;
}
void AdequacyCriterionBiLevel::SetLambdaMaxToMaxInvestmentCosts(
    const std::vector<double> &obj, const Point &max_invest,
    const VariableMap &master_variable) {
  lambda_max_ = 0;
  for (const auto &[var_name, var_id] : master_variable) {
    lambda_max_ += obj[var_id] * max_invest.at(var_name);
  }
}
