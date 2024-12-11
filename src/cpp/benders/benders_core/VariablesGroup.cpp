#include "include/antares-xpansion/benders/benders_core/VariablesGroup.h"

#include <regex>

using namespace Benders::Criterion;

/**
 * @file VariablesGroup.cpp
 * @brief Implementation of the VariablesGroup class.
 *
 * This file contains the implementation of the VariablesGroup class,
 * which is responsible for grouping variables based on provided input patterns.
 */

VariablesGroup::VariablesGroup(
    const std::vector<std::string>& all_variables,
    const std::vector<CriterionSingleInputData>& criterion_single_input_data)
    : all_variables_(all_variables), criterion_single_input_data_(criterion_single_input_data) {
  Search();
}

std::vector<std::vector<int>> VariablesGroup::Indices() const {
  return indices_;
}

void VariablesGroup::Search() {
  indices_.assign(criterion_single_input_data_.size(), {});
  int var_index(0);
  for (const auto& variable : all_variables_) {
    int pattern_index(0);
    for (const auto& single_input_data : criterion_single_input_data_) {
      if (std::regex_search(variable, single_input_data.Pattern().MakeRegex())) {
        indices_[pattern_index].push_back(var_index);
      }
      ++pattern_index;
    }
    ++var_index;
  }
}
