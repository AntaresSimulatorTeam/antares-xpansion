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

TEST(MergeMasterMPS, VariablePositionsGetSet)
{
    MergeMasterTrajectoryMPS::VariablePositions positions;
    positions.set(MergeMasterTrajectoryMPS::CAPACITY, 0);
    positions.set(MergeMasterTrajectoryMPS::DX_PLUS, 1);
    positions.set(MergeMasterTrajectoryMPS::DX_MINUS, 2);

    EXPECT_EQ(positions.get(MergeMasterTrajectoryMPS::CAPACITY), 0);
    EXPECT_EQ(positions.get(MergeMasterTrajectoryMPS::DX_PLUS), 1);
    EXPECT_EQ(positions.get(MergeMasterTrajectoryMPS::DX_MINUS), 2);
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

TEST(MergeMasterMPS, TrajectoryConstraintParsesCorrectly)
{
    Json::Value data;
    Json::Value coeffs;
    coeffs["node1::candidate1::x"] = 1.5;
    coeffs["node1::candidate1::dx_plus"] = 0.5;
    coeffs["node2::candidate2::dx_minus"] = 2.0;
    data[MasterStructureKeys::KEY_COEFFICIENTS] = coeffs;
    data[MasterStructureKeys::KEY_RHS] = 100.0;
    data[MasterStructureKeys::KEY_OPERATOR] = "=";

    MergeMasterTrajectoryMPS::TrajectoryConstraint constraint(data);

    EXPECT_DOUBLE_EQ(constraint.rhs, 100.0);
    EXPECT_EQ(constraint.constraint_type, 'E');
    EXPECT_EQ(constraint.coefficients_map.size(), 3);

    auto it1 = constraint.coefficients_map.find(
      {"node1", "candidate1", MergeMasterTrajectoryMPS::CAPACITY});
    ASSERT_NE(it1, constraint.coefficients_map.end());
    EXPECT_DOUBLE_EQ(it1->second, 1.5);

    auto it2 = constraint.coefficients_map.find(
      {"node1", "candidate1", MergeMasterTrajectoryMPS::DX_PLUS});
    ASSERT_NE(it2, constraint.coefficients_map.end());
    EXPECT_DOUBLE_EQ(it2->second, 0.5);

    auto it3 = constraint.coefficients_map.find(
      {"node2", "candidate2", MergeMasterTrajectoryMPS::DX_MINUS});
    ASSERT_NE(it3, constraint.coefficients_map.end());
    EXPECT_DOUBLE_EQ(it3->second, 2.0);
}

TEST(MergeMasterMPS, TrajectoryGlobalDataParsesInitialCapacities)
{
    Json::Value data;
    Json::Value initial_capacities;
    initial_capacities["default"] = 0.0;
    initial_capacities["candidate1"] = 100.0;
    initial_capacities["candidate2"] = 200.0;
    data[MasterStructureKeys::KEY_INITIAL_CAPACITIES] = initial_capacities;
    data[MasterStructureKeys::KEY_CONSTRAINTS] = Json::Value(Json::arrayValue);

    MergeMasterTrajectoryMPS::TrajectoryGlobalData global_data(data);

    EXPECT_EQ(global_data.initial_capacities.size(), 3);
    EXPECT_DOUBLE_EQ(global_data.initial_capacities["default"], 0.0);
    EXPECT_DOUBLE_EQ(global_data.initial_capacities["candidate1"], 100.0);
    EXPECT_DOUBLE_EQ(global_data.initial_capacities["candidate2"], 200.0);
}

TEST(MergeMasterMPS, TrajectoryGlobalDataParsesConstraints)
{
    Json::Value data;
    data[MasterStructureKeys::KEY_INITIAL_CAPACITIES] = Json::Value(Json::objectValue);

    Json::Value constraints(Json::arrayValue);
    Json::Value constraint1;
    Json::Value coeffs1;
    coeffs1["node1::candidate1::x"] = 1.0;
    constraint1[MasterStructureKeys::KEY_COEFFICIENTS] = coeffs1;
    constraint1[MasterStructureKeys::KEY_RHS] = 100.0;
    constraint1[MasterStructureKeys::KEY_OPERATOR] = "<";

    Json::Value constraint2;
    Json::Value coeffs2;
    coeffs2["node2::candidate2::x"] = 2.0;
    constraint2[MasterStructureKeys::KEY_COEFFICIENTS] = coeffs2;
    constraint2[MasterStructureKeys::KEY_RHS] = 200.0;
    constraint2[MasterStructureKeys::KEY_OPERATOR] = ">";

    constraints.append(constraint1);
    constraints.append(constraint2);
    data[MasterStructureKeys::KEY_CONSTRAINTS] = constraints;

    MergeMasterTrajectoryMPS::TrajectoryGlobalData global_data(data);

    EXPECT_EQ(global_data.trajectory_constraints.size(), 2);
    EXPECT_DOUBLE_EQ(global_data.trajectory_constraints[0].rhs, 100.0);
    EXPECT_EQ(global_data.trajectory_constraints[0].constraint_type, 'L');
    EXPECT_DOUBLE_EQ(global_data.trajectory_constraints[1].rhs, 200.0);
    EXPECT_EQ(global_data.trajectory_constraints[1].constraint_type, 'G');
}

TEST(MergeMasterMPS, TrajectoryNodeParsesCorrectly)
{
    Json::Value data;
    data[MasterStructureKeys::KEY_PARENT] = "parent_node";

    Json::Value candidates;
    Json::Value candidate1_costs;
    candidate1_costs[MasterStructureKeys::KEY_OPERATION_COST] = 10.0;
    candidate1_costs[MasterStructureKeys::KEY_INVESTMENT_COST] = 20.0;
    candidate1_costs[MasterStructureKeys::KEY_RETIREMENT_COST] = 5.0;
    candidates["candidate1"] = candidate1_costs;

    Json::Value candidate2_costs;
    candidate2_costs[MasterStructureKeys::KEY_OPERATION_COST] = 15.0;
    candidate2_costs[MasterStructureKeys::KEY_INVESTMENT_COST] = 25.0;
    candidate2_costs[MasterStructureKeys::KEY_RETIREMENT_COST] = 7.5;
    candidates["candidate2"] = candidate2_costs;

    data[MasterStructureKeys::KEY_CANDIDATES] = candidates;

    MergeMasterTrajectoryMPS::TrajectoryNode node("node1", data);

    EXPECT_EQ(node.name, "node1");
    ASSERT_TRUE(node.parent.has_value());
    EXPECT_EQ(node.parent.value(), "parent_node");
    EXPECT_EQ(node.candidates_costs.size(), 2);

    ASSERT_NE(node.candidates_costs.find("candidate1"), node.candidates_costs.end());
    EXPECT_DOUBLE_EQ(node.candidates_costs.at("candidate1").operation_maintenance, 10.0);
    EXPECT_DOUBLE_EQ(node.candidates_costs.at("candidate1").investment, 20.0);
    EXPECT_DOUBLE_EQ(node.candidates_costs.at("candidate1").retirement, 5.0);
}

TEST(MergeMasterMPS, TrajectoryNodeWithRootParentHasNoParent)
{
    Json::Value data;
    data[MasterStructureKeys::KEY_PARENT] = "root";

    Json::Value candidates;
    Json::Value candidate1_costs;
    candidate1_costs[MasterStructureKeys::KEY_OPERATION_COST] = 10.0;
    candidate1_costs[MasterStructureKeys::KEY_INVESTMENT_COST] = 20.0;
    candidate1_costs[MasterStructureKeys::KEY_RETIREMENT_COST] = 5.0;
    candidates["candidate1"] = candidate1_costs;

    data[MasterStructureKeys::KEY_CANDIDATES] = candidates;

    MergeMasterTrajectoryMPS::TrajectoryNode node("node1", data);

    EXPECT_EQ(node.name, "node1");
    EXPECT_FALSE(node.parent.has_value());
}

TEST(MergeMasterMPS, TrajectoryNodeWithoutParentHasNoParent)
{
    Json::Value data;

    Json::Value candidates;
    Json::Value candidate1_costs;
    candidate1_costs[MasterStructureKeys::KEY_OPERATION_COST] = 10.0;
    candidate1_costs[MasterStructureKeys::KEY_INVESTMENT_COST] = 20.0;
    candidate1_costs[MasterStructureKeys::KEY_RETIREMENT_COST] = 5.0;
    candidates["candidate1"] = candidate1_costs;

    data[MasterStructureKeys::KEY_CANDIDATES] = candidates;

    MergeMasterTrajectoryMPS::TrajectoryNode node("node1", data);

    EXPECT_EQ(node.name, "node1");
    EXPECT_FALSE(node.parent.has_value());
}

TEST(MergeMasterMPS, VariablePositionsSetAnd)
{
    MergeMasterTrajectoryMPS::VariablePositions positions;

    positions.set(MergeMasterTrajectoryMPS::CAPACITY, 10);
    positions.set(MergeMasterTrajectoryMPS::DX_PLUS, 11);
    positions.set(MergeMasterTrajectoryMPS::DX_MINUS, 12);

    EXPECT_EQ(positions.capacity, 10);
    EXPECT_EQ(positions.dx_plus, 11);
    EXPECT_EQ(positions.dx_minus, 12);
}

TEST(MergeMasterMPS, TrajectoryNodeDefaultConstructor)
{
    MergeMasterTrajectoryMPS::TrajectoryNode node;

    EXPECT_TRUE(node.name.empty());
    EXPECT_FALSE(node.parent.has_value());
    EXPECT_TRUE(node.candidates_costs.empty());
}

TEST(MergeMasterMPS, TrajectoryConstraintWithMultipleNodesAndCandidates)
{
    Json::Value data;
    Json::Value coeffs;

    coeffs["node_A::candidate_wind::x"] = 1.0;
    coeffs["node_A::candidate_solar::dx_plus"] = 2.5;
    coeffs["node_B::candidate_wind::dx_minus"] = -0.5;
    coeffs["node_C::candidate_gas::x"] = 3.0;

    data[MasterStructureKeys::KEY_COEFFICIENTS] = coeffs;
    data[MasterStructureKeys::KEY_RHS] = 500.0;
    data[MasterStructureKeys::KEY_OPERATOR] = ">";

    MergeMasterTrajectoryMPS::TrajectoryConstraint constraint(data);

    EXPECT_EQ(constraint.coefficients_map.size(), 4);
    EXPECT_DOUBLE_EQ(constraint.rhs, 500.0);
    EXPECT_EQ(constraint.constraint_type, 'G');

    auto wind_capacity_it = constraint.coefficients_map.find(
      {"node_A", "candidate_wind", MergeMasterTrajectoryMPS::CAPACITY});
    ASSERT_NE(wind_capacity_it, constraint.coefficients_map.end());
    EXPECT_DOUBLE_EQ(wind_capacity_it->second, 1.0);
}

TEST(MergeMasterMPS, CandidateCostsWithZeroCosts)
{
    Json::Value data;
    data[MasterStructureKeys::KEY_OPERATION_COST] = 0.0;
    data[MasterStructureKeys::KEY_INVESTMENT_COST] = 0.0;
    data[MasterStructureKeys::KEY_RETIREMENT_COST] = 0.0;

    MergeMasterTrajectoryMPS::CandidateCosts costs(data);

    EXPECT_DOUBLE_EQ(costs.operation_maintenance, 0.0);
    EXPECT_DOUBLE_EQ(costs.investment, 0.0);
    EXPECT_DOUBLE_EQ(costs.retirement, 0.0);
    EXPECT_DOUBLE_EQ(costs.get(MergeMasterTrajectoryMPS::CAPACITY), 0.0);
    EXPECT_DOUBLE_EQ(costs.get(MergeMasterTrajectoryMPS::DX_PLUS), 0.0);
    EXPECT_DOUBLE_EQ(costs.get(MergeMasterTrajectoryMPS::DX_MINUS), 0.0);
}

TEST(MergeMasterMPS, CandidateCostsWithLargeValues)
{
    Json::Value data;
    data[MasterStructureKeys::KEY_OPERATION_COST] = 1e10;
    data[MasterStructureKeys::KEY_INVESTMENT_COST] = 5e9;
    data[MasterStructureKeys::KEY_RETIREMENT_COST] = 2e8;

    MergeMasterTrajectoryMPS::CandidateCosts costs(data);

    EXPECT_DOUBLE_EQ(costs.get(MergeMasterTrajectoryMPS::CAPACITY), 1e10);
    EXPECT_DOUBLE_EQ(costs.get(MergeMasterTrajectoryMPS::DX_PLUS), 5e9);
    EXPECT_DOUBLE_EQ(costs.get(MergeMasterTrajectoryMPS::DX_MINUS), 2e8);
}

TEST(MergeMasterMPS, CandidateCostsWithNegativeInvestmentCost)
{
    Json::Value data;
    data[MasterStructureKeys::KEY_OPERATION_COST] = 100.0;
    data[MasterStructureKeys::KEY_INVESTMENT_COST] = -50.0;
    data[MasterStructureKeys::KEY_RETIREMENT_COST] = 25.0;

    MergeMasterTrajectoryMPS::CandidateCosts costs(data);

    EXPECT_DOUBLE_EQ(costs.investment, -50.0);
    EXPECT_DOUBLE_EQ(costs.get(MergeMasterTrajectoryMPS::DX_PLUS), -50.0);
}

TEST(MergeMasterMPS, TrajectoryGlobalDataWithNoConstraints)
{
    Json::Value data;
    Json::Value initial_capacities;
    initial_capacities["default"] = 0.0;
    data[MasterStructureKeys::KEY_INITIAL_CAPACITIES] = initial_capacities;
    data[MasterStructureKeys::KEY_CONSTRAINTS] = Json::Value(Json::arrayValue);

    MergeMasterTrajectoryMPS::TrajectoryGlobalData global_data(data);

    EXPECT_TRUE(global_data.trajectory_constraints.empty());
    EXPECT_EQ(global_data.initial_capacities.size(), 1);
}

TEST(MergeMasterMPS, TrajectoryGlobalDataWithSingleConstraint)
{
    Json::Value data;
    data[MasterStructureKeys::KEY_INITIAL_CAPACITIES] = Json::Value(Json::objectValue);

    Json::Value constraints(Json::arrayValue);
    Json::Value constraint;
    Json::Value coeffs;
    coeffs["n1::c1::x"] = 1.0;
    coeffs["n1::c1::dx_plus"] = 0.5;
    constraint[MasterStructureKeys::KEY_COEFFICIENTS] = coeffs;
    constraint[MasterStructureKeys::KEY_RHS] = 100.0;
    constraint[MasterStructureKeys::KEY_OPERATOR] = "=";
    constraints.append(constraint);
    data[MasterStructureKeys::KEY_CONSTRAINTS] = constraints;

    MergeMasterTrajectoryMPS::TrajectoryGlobalData global_data(data);

    EXPECT_EQ(global_data.trajectory_constraints.size(), 1);
    EXPECT_DOUBLE_EQ(global_data.trajectory_constraints[0].rhs, 100.0);
    EXPECT_EQ(global_data.trajectory_constraints[0].coefficients_map.size(), 2);
}

TEST(MergeMasterMPS, TrajectoryGlobalDataWithMultipleConstraintsAndOperators)
{
    Json::Value data;
    data[MasterStructureKeys::KEY_INITIAL_CAPACITIES] = Json::Value(Json::objectValue);

    Json::Value constraints(Json::arrayValue);

    Json::Value constraint1;
    Json::Value coeffs1;
    coeffs1["n1::c1::x"] = 1.0;
    constraint1[MasterStructureKeys::KEY_COEFFICIENTS] = coeffs1;
    constraint1[MasterStructureKeys::KEY_RHS] = 100.0;
    constraint1[MasterStructureKeys::KEY_OPERATOR] = "=";
    constraints.append(constraint1);

    Json::Value constraint2;
    Json::Value coeffs2;
    coeffs2["n2::c2::x"] = 2.0;
    constraint2[MasterStructureKeys::KEY_COEFFICIENTS] = coeffs2;
    constraint2[MasterStructureKeys::KEY_RHS] = 200.0;
    constraint2[MasterStructureKeys::KEY_OPERATOR] = "<";
    constraints.append(constraint2);

    Json::Value constraint3;
    Json::Value coeffs3;
    coeffs3["n3::c3::x"] = 3.0;
    constraint3[MasterStructureKeys::KEY_COEFFICIENTS] = coeffs3;
    constraint3[MasterStructureKeys::KEY_RHS] = 300.0;
    constraint3[MasterStructureKeys::KEY_OPERATOR] = ">";
    constraints.append(constraint3);

    data[MasterStructureKeys::KEY_CONSTRAINTS] = constraints;

    MergeMasterTrajectoryMPS::TrajectoryGlobalData global_data(data);

    EXPECT_EQ(global_data.trajectory_constraints.size(), 3);
    EXPECT_EQ(global_data.trajectory_constraints[0].constraint_type, 'E');
    EXPECT_EQ(global_data.trajectory_constraints[1].constraint_type, 'L');
    EXPECT_EQ(global_data.trajectory_constraints[2].constraint_type, 'G');
}

TEST(MergeMasterMPS, TrajectoryNodeWithSingleCandidate)
{
    Json::Value data;

    Json::Value candidates;
    Json::Value candidate_costs;
    candidate_costs[MasterStructureKeys::KEY_OPERATION_COST] = 50.0;
    candidate_costs[MasterStructureKeys::KEY_INVESTMENT_COST] = 100.0;
    candidate_costs[MasterStructureKeys::KEY_RETIREMENT_COST] = 25.0;
    candidates["wind_farm"] = candidate_costs;

    data[MasterStructureKeys::KEY_CANDIDATES] = candidates;

    MergeMasterTrajectoryMPS::TrajectoryNode node("root", data);

    EXPECT_EQ(node.name, "root");
    EXPECT_EQ(node.candidates_costs.size(), 1);

    auto it = node.candidates_costs.find("wind_farm");
    ASSERT_NE(it, node.candidates_costs.end());
    EXPECT_DOUBLE_EQ(it->second.operation_maintenance, 50.0);
    EXPECT_DOUBLE_EQ(it->second.investment, 100.0);
    EXPECT_DOUBLE_EQ(it->second.retirement, 25.0);
}

TEST(MergeMasterMPS, TrajectoryNodeWithMultipleCandidatesWithDifferentCosts)
{
    Json::Value data;

    Json::Value candidates;

    Json::Value wind_costs;
    wind_costs[MasterStructureKeys::KEY_OPERATION_COST] = 30.0;
    wind_costs[MasterStructureKeys::KEY_INVESTMENT_COST] = 150.0;
    wind_costs[MasterStructureKeys::KEY_RETIREMENT_COST] = 20.0;
    candidates["wind"] = wind_costs;

    Json::Value solar_costs;
    solar_costs[MasterStructureKeys::KEY_OPERATION_COST] = 10.0;
    solar_costs[MasterStructureKeys::KEY_INVESTMENT_COST] = 100.0;
    solar_costs[MasterStructureKeys::KEY_RETIREMENT_COST] = 15.0;
    candidates["solar"] = solar_costs;

    Json::Value storage_costs;
    storage_costs[MasterStructureKeys::KEY_OPERATION_COST] = 5.0;
    storage_costs[MasterStructureKeys::KEY_INVESTMENT_COST] = 200.0;
    storage_costs[MasterStructureKeys::KEY_RETIREMENT_COST] = 30.0;
    candidates["storage"] = storage_costs;

    data[MasterStructureKeys::KEY_CANDIDATES] = candidates;

    MergeMasterTrajectoryMPS::TrajectoryNode node("node_2025", data);

    EXPECT_EQ(node.candidates_costs.size(), 3);

    EXPECT_DOUBLE_EQ(node.candidates_costs.at("wind").investment, 150.0);
    EXPECT_DOUBLE_EQ(node.candidates_costs.at("solar").investment, 100.0);
    EXPECT_DOUBLE_EQ(node.candidates_costs.at("storage").investment, 200.0);
}

TEST(MergeMasterMPS, VariablePositionsOverwriteValues)
{
    MergeMasterTrajectoryMPS::VariablePositions positions;

    positions.set(MergeMasterTrajectoryMPS::CAPACITY, 1);
    positions.set(MergeMasterTrajectoryMPS::DX_PLUS, 2);
    positions.set(MergeMasterTrajectoryMPS::DX_MINUS, 3);

    EXPECT_EQ(positions.get(MergeMasterTrajectoryMPS::CAPACITY), 1);
    EXPECT_EQ(positions.get(MergeMasterTrajectoryMPS::DX_PLUS), 2);
    EXPECT_EQ(positions.get(MergeMasterTrajectoryMPS::DX_MINUS), 3);

    positions.set(MergeMasterTrajectoryMPS::CAPACITY, 10);
    positions.set(MergeMasterTrajectoryMPS::DX_PLUS, 20);

    EXPECT_EQ(positions.get(MergeMasterTrajectoryMPS::CAPACITY), 10);
    EXPECT_EQ(positions.get(MergeMasterTrajectoryMPS::DX_PLUS), 20);
    EXPECT_EQ(positions.get(MergeMasterTrajectoryMPS::DX_MINUS), 3);
}

TEST(MergeMasterMPS, TrajectoryConstraintWithZeroCoefficients)
{
    Json::Value data;
    Json::Value coeffs;

    coeffs["node1::candidate1::x"] = 0.0;
    coeffs["node1::candidate1::dx_plus"] = 0.0;
    coeffs["node2::candidate2::dx_minus"] = 1.5;

    data[MasterStructureKeys::KEY_COEFFICIENTS] = coeffs;
    data[MasterStructureKeys::KEY_RHS] = 50.0;
    data[MasterStructureKeys::KEY_OPERATOR] = "<";

    MergeMasterTrajectoryMPS::TrajectoryConstraint constraint(data);

    EXPECT_EQ(constraint.coefficients_map.size(), 3);
    EXPECT_EQ(constraint.constraint_type, 'L');

    auto zero_coef_it = constraint.coefficients_map.find(
      {"node1", "candidate1", MergeMasterTrajectoryMPS::CAPACITY});
    ASSERT_NE(zero_coef_it, constraint.coefficients_map.end());
    EXPECT_DOUBLE_EQ(zero_coef_it->second, 0.0);
}
