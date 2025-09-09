#include "antares-xpansion/benders/benders_core/CriterionComputation.h"

#include <iostream>

namespace Benders::Criterion
{

void CriterionComputation::ComputeCriterion(double subproblem_weight,
                                            const std::vector<double>& sub_problem_solution,
                                            std::vector<double>& criteria,
                                            std::vector<double>& patterns_values)
{
    auto criteria_input_size = static_cast<int>(var_indices_.size()); // num of patterns
    criteria.resize(criteria_input_size, 0.);
    patterns_values.resize(criteria_input_size, 0.);

    double criterion_count_threshold = criterion_input_data_.CriterionCountThreshold();

    for (int pattern_index(0); pattern_index < criteria_input_size; ++pattern_index)
    {
        auto pattern_variables_indices = var_indices_[pattern_index];
        double pattern_value = patterns_values[pattern_index];
        double criteria_value = criteria[pattern_index];
        for (auto variables_index: pattern_variables_indices)
        {
            const auto solution = sub_problem_solution[variables_index];
            pattern_value += solution;
            if (solution > criterion_count_threshold)
            {
                // 1h of no supplied energy
                criteria_value += subproblem_weight;
            }
        }
        patterns_values[pattern_index] = pattern_value;
        criteria[pattern_index] = criteria_value;
    }
}

void CriterionComputation::SearchVariables(const std::vector<std::string>& variables)
{
    Benders::Criterion::VariablesGroup variablesGroup(variables, criterion_input_data_.Criteria());
    var_indices_ = variablesGroup.Indices();
}

const CriterionInputData& CriterionComputation::getCriterionInputData() const
{
    return criterion_input_data_;
}

std::vector<std::vector<int>>& CriterionComputation::getVarIndices()
{
    std::cout << "[DEBUG][getVarIndices] var_indices_ size=" << var_indices_.size() << std::endl;
    for (size_t i = 0; i < var_indices_.size(); ++i)
    {
        std::cout << "[DEBUG][getVarIndices] var_indices_[" << i
                  << "] size=" << var_indices_[i].size() << ", values=";
        for (size_t j = 0; j < var_indices_[i].size(); ++j)
        {
            std::cout << var_indices_[i][j] << " ";
        }
        std::cout << std::endl;
    }
    return var_indices_;
}

CriterionComputation::CriterionComputation(const CriterionInputData& criterion_input_data):
    criterion_input_data_(criterion_input_data)
{
}
} // namespace Benders::Criterion
