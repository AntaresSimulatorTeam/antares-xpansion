#pragma once

#include "OuterLoopInputDataReader.h"
#include "VariablesGroup.h"
namespace Benders::Criterion {

class CriterionComputation {
 public:
  /**
   * @brief Constructs a CriterionComputation object.
   *
   * This constructor initializes the CriterionComputation instance with the
   * provided outer loop input data.
   *
   * @param outer_loop_input_data The input data to be used for criterion
   * computation.
   */
  explicit CriterionComputation(
      const OuterLoopInputData &outer_loop_input_data);

  /**
   * @brief Searches for relevant variables based on the provided variable
   * names.
   *
   * This method initializes a VariablesGroup with the provided variable names
   * and retrieves the indices of these variables for later computation.
   *
   * @param variables A vector of strings representing the variable names to
   * search for.
   */
  void SearchVariables(const std::vector<std::string> &variables);

  /**
   * @brief Computes the outer loop criterion based on subproblem solutions.
   *
   * This method calculates the outer loop criteria and pattern values
   * based on the provided subproblem weight and solution. It updates the
   * outerLoopCriterions and outerLoopPatternsValues vectors accordingly.
   *
   * @param subproblem_weight The weight of the subproblem affecting the
   * criteria.
   * @param sub_problem_solution A vector containing the solutions of the
   * subproblem.
   * @param outerLoopCriterions A reference to a vector where the computed
   * criteria will be stored.
   * @param outerLoopPatternsValues A reference to a vector where the computed
   * pattern values will be stored.
   */
  void ComputeOuterLoopCriterion(
      double subproblem_weight, const std::vector<double> &sub_problem_solution,
      std::vector<double> &outerLoopCriterions,
      std::vector<double> &outerLoopPatternsValues);

  /**
   * @brief Retrieves the variable indices.
   *
   * This method returns a reference to the vector containing the indices of
   * the variables associated with this CriterionComputation instance.
   *
   * @return A reference to the vector of variable indices.
   */
  std::vector<std::vector<int>> &getVarIndices();

  /**
   * @brief Retrieves the outer loop input data.
   *
   * This method returns a constant reference to the outer loop input data
   * associated with this CriterionComputation instance.
   *
   * @return A constant reference to the OuterLoopInputData object.
   */
  const OuterLoopInputData &getOuterLoopInputData() const;

 private:
  std::vector<std::vector<int>> var_indices_;
  const OuterLoopInputData outer_loop_input_data_;
};
}  // namespace Benders::Criterion