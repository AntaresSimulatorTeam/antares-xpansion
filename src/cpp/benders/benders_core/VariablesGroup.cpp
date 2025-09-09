#include "include/antares-xpansion/benders/benders_core/VariablesGroup.h"

#include <iostream>
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
  const std::vector<CriterionSingleInputData>& criterion_single_input_data):
    all_variables_(all_variables),
    criterion_single_input_data_(criterion_single_input_data)
{
    std::cout << "[DEBUG][VariablesGroup] Constructor: all_variables size=" << all_variables.size()
              << std::endl;
    for (size_t i = 0; i < all_variables.size(); ++i)
    {
        std::cout << "[DEBUG][VariablesGroup] all_variables[" << i << "]=" << all_variables[i]
                  << std::endl;
    }
    std::cout << "[DEBUG][VariablesGroup] criterion_single_input_data size="
              << criterion_single_input_data.size() << std::endl;
    for (size_t i = 0; i < criterion_single_input_data.size(); ++i)
    {
        std::cout << "[DEBUG][VariablesGroup] pattern[" << i
                  << "]=" << criterion_single_input_data[i].Pattern().Value() << std::endl;
    }
    Search();
    for (size_t i = 0; i < indices_.size(); ++i)
    {
        std::cout << "[DEBUG][VariablesGroup] indices_[" << i << "] size=" << indices_[i].size()
                  << ", values=";
        for (size_t j = 0; j < indices_[i].size(); ++j)
        {
            std::cout << indices_[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

std::vector<std::vector<int>> VariablesGroup::Indices() const
{
    return indices_;
}

void VariablesGroup::Search()
{
    indices_.assign(criterion_single_input_data_.size(), {});
    int pattern_index(0);
    for (const auto& single_input_data: criterion_single_input_data_)
    {
        auto pattern = single_input_data.Pattern().Value();
        int var_index(0);
        for (const auto& variable: all_variables_)
        {
            if (variable.starts_with(pattern))
            {
                indices_[pattern_index].push_back(var_index);
            }
            ++var_index;
        }
        ++pattern_index;
    }
}
