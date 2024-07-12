#include "VariablesGroup.h"
using namespace AdequacyCriterionSpace;

VariablesGroup::VariablesGroup(
    const std::vector<std::string>& all_variables,
    const std::vector<AdequacyCriterionSingleInputData>&
        adequacy_criterion_single_input_data)
    : all_variables_(all_variables),
      adequacy_criterion_single_input_data_(
          adequacy_criterion_single_input_data) {
  Search();
}

std::vector<std::vector<int>> VariablesGroup::Indices() const {
  return indices_;
}

void VariablesGroup::Search() {
  indices_.assign(adequacy_criterion_single_input_data_.size(), {});
  int var_index(0);
  for (const auto& variable : all_variables_) {
    int pattern_index(0);
    for (const auto& single_input_data :
         adequacy_criterion_single_input_data_) {
      if (std::regex_search(variable, single_input_data.Pattern().MakeRegex())) {
        indices_[pattern_index].push_back(var_index);
      }
      ++pattern_index;
    }
    ++var_index;
  }
}
