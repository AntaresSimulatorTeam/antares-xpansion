#include "antares-xpansion/benders/merge_master_mps/MergeMasterMPS.h"
#include "gtest/gtest.h"

TEST(MergeMasterMPS, ParseVariableType)
{
    EXPECT_EQ(MergeMasterTrajectoryMPS::parse_variable_type("x"),
              MergeMasterTrajectoryMPS::CAPACITY);
    EXPECT_EQ(MergeMasterTrajectoryMPS::parse_variable_type("dx_plus"),
              MergeMasterTrajectoryMPS::DX_PLUS);
    EXPECT_EQ(MergeMasterTrajectoryMPS::parse_variable_type("dx_minus"),
              MergeMasterTrajectoryMPS::DX_MINUS);
}

TEST(MergeMasterMPS, ParseConstraintType)
{
    EXPECT_EQ(MergeMasterTrajectoryMPS::parse_constraint_type("="), 'E');
    EXPECT_EQ(MergeMasterTrajectoryMPS::parse_constraint_type(">"), 'G');
    EXPECT_EQ(MergeMasterTrajectoryMPS::parse_constraint_type("<"), 'L');
}

TEST(MergeMasterMPS, VariablePositionsGet)
{
    MergeMasterTrajectoryMPS::VariablePositions positions;
    positions.set(MergeMasterTrajectoryMPS::CAPACITY, 0);
    positions.set(MergeMasterTrajectoryMPS::DX_PLUS, 1);
    positions.set(MergeMasterTrajectoryMPS::DX_MINUS, 2);

    EXPECT_EQ(positions.get(MergeMasterTrajectoryMPS::CAPACITY), 0);
    EXPECT_EQ(positions.get(MergeMasterTrajectoryMPS::DX_PLUS), 1);
    EXPECT_EQ(positions.get(MergeMasterTrajectoryMPS::DX_MINUS), 2);
}

TEST(MergeMasterMPS, VariablePositionsSet)
{
    MergeMasterTrajectoryMPS::VariablePositions positions;
    positions.set(MergeMasterTrajectoryMPS::CAPACITY, 5);
    positions.set(MergeMasterTrajectoryMPS::DX_PLUS, 10);
    positions.set(MergeMasterTrajectoryMPS::DX_MINUS, 15);

    EXPECT_EQ(positions.get(MergeMasterTrajectoryMPS::CAPACITY), 5);
    EXPECT_EQ(positions.get(MergeMasterTrajectoryMPS::DX_PLUS), 10);
    EXPECT_EQ(positions.get(MergeMasterTrajectoryMPS::DX_MINUS), 15);
}

TEST(MergeMasterMPS, CandidateCostsGet)
{
    Json::Value data;
    data[MasterStructureKeys::KEY_OPERATION_COST] = 100.0;
    data[MasterStructureKeys::KEY_INVESTMENT_COST] = 200.0;
    data[MasterStructureKeys::KEY_RETIREMENT_COST] = 50.0;

    MergeMasterTrajectoryMPS::CandidateCosts costs(data);

    EXPECT_DOUBLE_EQ(costs.get(MergeMasterTrajectoryMPS::CAPACITY), 100.0);
    EXPECT_DOUBLE_EQ(costs.get(MergeMasterTrajectoryMPS::DX_PLUS), 200.0);
    EXPECT_DOUBLE_EQ(costs.get(MergeMasterTrajectoryMPS::DX_MINUS), 50.0);
}
