#pragma once
#include <regex>
#include <string>
#include <vector>

#include "CriterionInputDataReader.h"
namespace Benders::Criterion {
class VariablesGroup {
 public:
  explicit VariablesGroup(const std::vector<std::string>& all_variables,
                          const std::vector<CriterionSingleInputData>&
                          criterion_single_input_data);

  [[nodiscard]] std::vector<std::vector<int>> Indices() const;

 private:
  void Search();
  const std::vector<std::string>& all_variables_;
  const std::vector<CriterionSingleInputData> &criterion_single_input_data_;
  std::vector<std::vector<int>> indices_;
};
}  // namespace Benders::Criterion