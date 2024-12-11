#include "antares-xpansion/benders/benders_core/CriterionComputation.h"

namespace Benders::Criterion {

void CriterionComputation::ComputeCriterion(
    double subproblem_weight, const std::vector<double> &sub_problem_solution,
    std::vector<double> &criteria,
    std::vector<double> &patterns_values) {
    auto criteria_input_size = var_indices_.size(); // num of patterns
    criteria.resize(criteria_input_size, 0.);
    patterns_values.resize(criteria_input_size, 0.);

    double criterion_count_threshold =
            criterion_input_data_.CriterionCountThreshold();

    for (int pattern_index(0); pattern_index < criteria_input_size;
         ++pattern_index) {
        auto pattern_variables_indices = var_indices_[pattern_index];
        for (auto variables_index: pattern_variables_indices) {
            const auto &solution = sub_problem_solution[variables_index];
            patterns_values[pattern_index] += solution;
            if (solution > criterion_count_threshold)
                // 1h of no supplied energy
                criteria[pattern_index] += subproblem_weight;
        }
    }
}

void CriterionComputation::SearchVariables(
    const std::vector<std::string> &variables) {
  Benders::Criterion::VariablesGroup variablesGroup(
      variables, criterion_input_data_.Criteria());
  var_indices_ = variablesGroup.Indices();
}

const CriterionInputData &CriterionComputation::getCriterionInputData() const {
    return criterion_input_data_;
}

std::vector<std::vector<int>> &CriterionComputation::getVarIndices() {
  return var_indices_;
}
CriterionComputation::CriterionComputation(
    const CriterionInputData &criterion_input_data)
    : criterion_input_data_(criterion_input_data) {
}
}  // namespace Benders::Criterion