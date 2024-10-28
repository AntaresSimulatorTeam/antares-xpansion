//
// Created by mitripet on 24/10/24.
//

#include <gtest/gtest.h>

#include "LoggerBuilder.h"
#include "antares-xpansion/lpnamer/problem_modifier/FileProblemProviderAdapter.h"
#include "antares-xpansion/lpnamer/problem_modifier/ProblemVariablesFromProblemAdapter.h"

std::string mpsWithIndexStartingAt0() {
  return R"(NAME MINIMIP
ROWS
    N                                                   OBJ
COLUMNS
    NTCDirect::link<area1$$area2>::v0<0>                OBJ     -5.0
    NTCDirect::link<area1$$area2>::v1<1>                OBJ     -4.0
    IntercoDirectCost::link<area1$$area2>::v2<2>        OBJ     -5.0
    IntercoDirectCost::link<area1$$area2>::v3<3>        OBJ     -4.0
    IntercoIndirectCost::link<area1$$area2>::v4<1>      OBJ     -5.0
    IntercoIndirectCost::link<area1$$area2>::v5<3>      OBJ     -4.0
ENDATA)";
}

std::string mpsWithIndexStartingAt168() {
  return R"(NAME MINIMIP
ROWS
    N                                                     OBJ
COLUMNS
    NTCDirect::link<area1$$area2>::v0<168>                OBJ     -5.0
    NTCDirect::link<area1$$area2>::v1<169>                OBJ     -4.0
    IntercoDirectCost::link<area1$$area2>::v2<170>        OBJ     -5.0
    IntercoDirectCost::link<area1$$area2>::v3<171>        OBJ     -4.0
    IntercoIndirectCost::link<area1$$area2>::v4<169>      OBJ     -5.0
    IntercoIndirectCost::link<area1$$area2>::v5<171>      OBJ     -4.0
ENDATA)";
}

ProblemVariables provideVariables(std::string problem_name, std::string mps) {
  auto mps_path = std::filesystem::temp_directory_path() / problem_name;
  std::ofstream writer(mps_path);
  writer << mps;
  writer.close();
  auto solverLogger = SolverLogManager();
  FileProblemProviderAdapter problem_provider(mps_path, problem_name);
  auto problem = problem_provider.provide_problem("XPRESS", solverLogger);
  auto logger = emptyLogger();
  ActiveLink al(0, "area1 - area2", "area1", "area2", 0, logger);
  ProblemVariablesFromProblemAdapter variables_provider(problem, {al},
                                                        emptyLogger());
  return variables_provider.Provide();
}

void checkVarTimeSteps(ProblemVariables vars,
                       std::vector<int> expected_timesteps) {
  EXPECT_EQ(vars.ntc_columns.size(), 1);
  EXPECT_EQ(vars.ntc_columns.at(0).size(), 2);
  EXPECT_EQ(vars.ntc_columns.at(0).at(0),
            ColumnToChange(0, expected_timesteps[0]));
  EXPECT_EQ(vars.ntc_columns.at(0).at(1),
            ColumnToChange(1, expected_timesteps[1]));

  EXPECT_EQ(vars.direct_cost_columns.size(), 1);
  EXPECT_EQ(vars.direct_cost_columns.at(0).size(), 2);
  EXPECT_EQ(vars.direct_cost_columns.at(0).at(0),
            ColumnToChange(2, expected_timesteps[2]));
  EXPECT_EQ(vars.direct_cost_columns.at(0).at(1),
            ColumnToChange(3, expected_timesteps[3]));

  EXPECT_EQ(vars.indirect_cost_columns.size(), 1);
  EXPECT_EQ(vars.indirect_cost_columns.at(0).size(), 2);
  EXPECT_EQ(vars.indirect_cost_columns.at(0).at(0),
            ColumnToChange(4, expected_timesteps[4]));
  EXPECT_EQ(vars.indirect_cost_columns.at(0).at(1),
            ColumnToChange(5, expected_timesteps[5]));
}

TEST(ProblemVariablesFromProblemAdapterTest, ProvideVariablesForWeek1) {
  auto vars = provideVariables("problem-1-1--optim-nb-1.mps",
                               mpsWithIndexStartingAt0());
  checkVarTimeSteps(vars, {0, 1, 2, 3, 1, 3});
}

TEST(ProblemVariablesFromProblemAdapterTest,
     ProvideVariablesForWeek2StartingAt0) {
  auto vars = provideVariables("problem-1-2--optim-nb-2.mps",
                               mpsWithIndexStartingAt0());
  checkVarTimeSteps(vars, {168, 169, 170, 171, 169, 171});
}

TEST(ProblemVariablesFromProblemAdapterTest,
     ProvideVariablesForWeek2StartingAt168) {
  auto vars = provideVariables("problem-1-2--optim-nb-1.mps",
                               mpsWithIndexStartingAt168());
  checkVarTimeSteps(vars, {168, 169, 170, 171, 169, 171});
}

TEST(ProblemVariablesFromProblemAdapterTest,
     ProvideVariablesForWeek27StartingAt0) {
  auto vars = provideVariables("problem-1-27--optim-nb-2.mps",
                               mpsWithIndexStartingAt0());
  checkVarTimeSteps(vars, {4368, 4369, 4370, 4371, 4369, 4371});
}