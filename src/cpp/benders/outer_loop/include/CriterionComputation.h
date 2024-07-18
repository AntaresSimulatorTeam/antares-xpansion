#pragma once

#include "OuterLoopInputDataReader.h"
#include "VariablesGroup.h"
namespace Outerloop {
class CriterionComputation {
 public:
  CriterionComputation() = default;
  void LoadData(const std::filesystem::path &outer_loop_input_file);
  void SearchVariables(const std::vector<std::string> &variables);
  // outer loop criterion per pattern
  void ComputeOuterLoopCriterion(
      double subproblem_weight, const std::vector<double> &sub_problem_solution,
      std::vector<double> &outerLoopCriterions,
      std::vector<double> &outerLoopPatternsValues);
  std::vector<std::vector<int>> &getVarIndices();
  const OuterLoopInputData &getOuterLoopInputData() const;

 private:
  std::vector<std::vector<int>> var_indices_;

 private:
  OuterLoopInputData outer_loop_input_data_;
};
}  // namespace Outerloop