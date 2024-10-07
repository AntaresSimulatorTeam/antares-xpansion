#include "VariablesGroup.h"
#include <regex>


using namespace Outerloop;

/**
 * @file VariablesGroup.cpp
 * @brief Implementation of the VariablesGroup class.
 *
 * This file contains the implementation of the VariablesGroup class, 
 * which is responsible for grouping variables based on provided input patterns.
 */

VariablesGroup::VariablesGroup(
    const std::vector<std::string>& all_variables,
    const std::vector<OuterLoopSingleInputData>& outer_loop_single_input_data)
    : all_variables_(all_variables), outer_loop_single_input_data_(outer_loop_single_input_data) {
  Search();
}

std::vector<std::vector<int>> VariablesGroup::Indices() const {
  return indices_;
}

void VariablesGroup::Search() {
  indices_.assign(outer_loop_single_input_data_.size(), {});
  int var_index(0);
  for (const auto& variable : all_variables_) {
    int pattern_index(0);
    for (const auto& single_input_data : outer_loop_single_input_data_) {
      if (std::regex_search(variable, single_input_data.Pattern().MakeRegex())) {
        indices_[pattern_index].push_back(var_index);
      }
      ++pattern_index;
    }
    ++var_index;
  }
}
