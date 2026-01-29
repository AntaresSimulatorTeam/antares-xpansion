#include <gtest/gtest.h>
#include "antares-xpansion/benders/benders_core/CriterionComputation.h"
#include "antares-xpansion/benders/benders_core/CriterionInputDataReader.h"
using namespace Benders::Criterion;
class CriterionComputationTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        criterion_input_data_.SetCriterionCountThreshold(0.5);
        CriterionSingleInputData pattern1(PositiveUnsuppliedEnergy, "area1", 1.0);
        criterion_input_data_.AddSingleData(pattern1);
        CriterionSingleInputData pattern2(PositiveUnsuppliedEnergy, "area2", 1.0);
        criterion_input_data_.AddSingleData(pattern2);
    }
    CriterionInputData criterion_input_data_;
};
TEST_F(CriterionComputationTest, DefaultConstructor)
{
    CriterionComputation computation;
    EXPECT_TRUE(computation.IsEmpty());
}
TEST_F(CriterionComputationTest, ConstructorWithInputData)
{
    CriterionComputation computation(criterion_input_data_);
    EXPECT_FALSE(computation.IsEmpty());
    EXPECT_EQ(computation.getCriterionInputData().Criteria().size(), 2);
}
TEST_F(CriterionComputationTest, SearchVariablesBasic)
{
    CriterionComputation computation(criterion_input_data_);
    // Pattern format is "prefix + area<body>" = "PositiveUnsuppliedEnergy::area<area1>"
    std::vector<std::string> variables = {
        "PositiveUnsuppliedEnergy::area<area1>::hourly::0",
        "PositiveUnsuppliedEnergy::area<area1>::hourly::1",
        "PositiveUnsuppliedEnergy::area<area1>::hourly::2",
        "PositiveUnsuppliedEnergy::area<area2>::hourly::0",
        "PositiveUnsuppliedEnergy::area<area2>::hourly::1",
        "other_variable"
    };
    computation.SearchVariables(variables);
    auto& var_indices = computation.getVarIndices();
    EXPECT_EQ(var_indices.size(), 2);
    EXPECT_EQ(var_indices[0].size(), 3);
    EXPECT_EQ(var_indices[1].size(), 2);
}
TEST_F(CriterionComputationTest, ComputeCriterionBasic)
{
    CriterionComputation computation(criterion_input_data_);
    std::vector<std::string> variables = {
        "PositiveUnsuppliedEnergy::area<area1>::hourly::0",
        "PositiveUnsuppliedEnergy::area<area1>::hourly::1",
        "PositiveUnsuppliedEnergy::area<area2>::hourly::0"
    };
    computation.SearchVariables(variables);
    std::vector<double> sub_problem_solution = {1.5, 0.3, 2.0};
    double subproblem_weight = 1.0;
    std::vector<double> criteria;
    std::vector<double> patterns_values;
    computation.ComputeCriterion(subproblem_weight, sub_problem_solution, criteria, patterns_values);
    ASSERT_EQ(criteria.size(), 2);
    ASSERT_EQ(patterns_values.size(), 2);
    EXPECT_DOUBLE_EQ(patterns_values[0], 1.8);
    EXPECT_DOUBLE_EQ(criteria[0], 1.0);
}
TEST_F(CriterionComputationTest, ComputeCriterionWithHighValues)
{
    CriterionComputation computation(criterion_input_data_);
    std::vector<std::string> variables = {
        "PositiveUnsuppliedEnergy::area<area1>::hourly::0",
        "PositiveUnsuppliedEnergy::area<area1>::hourly::1"
    };
    computation.SearchVariables(variables);
    // Both values exceed threshold
    std::vector<double> sub_problem_solution = {1.0, 2.0};
    double subproblem_weight = 0.5;
    std::vector<double> criteria;
    std::vector<double> patterns_values;
    computation.ComputeCriterion(subproblem_weight, sub_problem_solution, criteria, patterns_values);
    // Both values exceed threshold (0.5), so criteria = 2 * 0.5 = 1.0
    EXPECT_DOUBLE_EQ(criteria[0], 1.0);
    EXPECT_DOUBLE_EQ(patterns_values[0], 3.0);
}
TEST_F(CriterionComputationTest, ComputeCriterionBelowThreshold)
{
    CriterionComputation computation(criterion_input_data_);
    std::vector<std::string> variables = {
        "PositiveUnsuppliedEnergy::area<area1>::hourly::0",
        "PositiveUnsuppliedEnergy::area<area1>::hourly::1",
        "PositiveUnsuppliedEnergy::area<area1>::hourly::2"
    };
    computation.SearchVariables(variables);
    // All values below threshold (0.5)
    std::vector<double> sub_problem_solution = {0.1, 0.2, 0.3};
    double subproblem_weight = 1.0;
    std::vector<double> criteria;
    std::vector<double> patterns_values;
    computation.ComputeCriterion(subproblem_weight, sub_problem_solution, criteria, patterns_values);
    // No values exceed threshold, so criteria = 0
    EXPECT_DOUBLE_EQ(criteria[0], 0.0);
    EXPECT_DOUBLE_EQ(patterns_values[0], 0.6);
}
