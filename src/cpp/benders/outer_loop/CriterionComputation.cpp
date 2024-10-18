#include "antares-xpansion/benders/outer_loop/CriterionComputation.h"

namespace Outerloop {

void CriterionComputation::ComputeOuterLoopCriterion(
    double subproblem_weight, const std::vector<double> &sub_problem_solution,
    std::vector<double> &outerLoopCriterions,
    std::vector<double> &outerLoopPatternsValues) {
  auto outer_loop_input_size = var_indices_.size();  // num of patterns
  outerLoopCriterions.resize(outer_loop_input_size, 0.);
  outerLoopPatternsValues.resize(outer_loop_input_size, 0.);

  double criterion_count_threshold =
      outer_loop_input_data_.CriterionCountThreshold();

  for (int pattern_index(0); pattern_index < outer_loop_input_size;
       ++pattern_index) {
    auto pattern_variables_indices = var_indices_[pattern_index];
    for (auto variables_index : pattern_variables_indices) {
      const auto &solution = sub_problem_solution[variables_index];
      outerLoopPatternsValues[pattern_index] += solution;
      if (solution > criterion_count_threshold)
        // 1h of no supplied energy
        outerLoopCriterions[pattern_index] += subproblem_weight;
    }
  }
}

void CriterionComputation::SearchVariables(
    const std::vector<std::string> &variables) {
  Outerloop::VariablesGroup variablesGroup(
      variables, outer_loop_input_data_.OuterLoopData());
  var_indices_ = variablesGroup.Indices();
}

const OuterLoopInputData &CriterionComputation::getOuterLoopInputData() const {
  return outer_loop_input_data_;
}

std::vector<std::vector<int>> &CriterionComputation::getVarIndices() {
  return var_indices_;
}
CriterionComputation::CriterionComputation(
    const OuterLoopInputData &outer_loop_input_data)
    : outer_loop_input_data_(outer_loop_input_data) {}

}  // namespace Outerloop