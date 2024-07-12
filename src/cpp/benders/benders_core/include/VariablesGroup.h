#pragma once
#include <regex>
#include <string>
#include <vector>

#include "AdequacyCriterionInputDataReader.h"
namespace AdequacyCriterionSpace {
class VariablesGroup {
 public:
  explicit VariablesGroup(const std::vector<std::string>& all_variables,
                          const std::vector<AdequacyCriterionSingleInputData>&
                              adequacy_criterion_single_input_data);
  [[nodiscard]] std::vector<std::vector<int>> Indices() const;

 private:
  void Search();
  const std::vector<std::string>& all_variables_;
  const std::vector<AdequacyCriterionSingleInputData>&
      adequacy_criterion_single_input_data_;
  std::vector<std::vector<int>> indices_;
};
}  // namespace AdequacyCriterionSpace