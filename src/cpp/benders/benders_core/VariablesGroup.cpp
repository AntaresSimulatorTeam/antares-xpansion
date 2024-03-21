#include "VariablesGroup.h"

VariablesGroup::VariablesGroup(const std::vector<std::string>& all_variables,
                               const std::vector<std::regex>& patterns)
    : all_variables_(all_variables), patterns_(patterns) {
  Search();
}

void VariablesGroup::Search() {
  indices_.assign(patterns_.size(), {});
  int var_index(0);
  for (const auto& variable : all_variables_) {
    int pattern_index(0);
    for (const auto& pattern : patterns_) {
      if (std::regex_search(variable, pattern)) {
        indices_[pattern_index].push_back(var_index);
      }
      ++pattern_index;
    }
    ++var_index;
  }
}