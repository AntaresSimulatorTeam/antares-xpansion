//
// Created by marechaljas on 02/05/2022.
//

#include <gtest/gtest.h>
#include <filesystem>
#include "antares-xpansion/lpnamer/model/ProblemNameParser.h"
#include "antares-xpansion/lpnamer/model/Problem.h"
#include "NOOPSolver.h"

struct ProblemNameCase {
  std::string problem_name;
  unsigned int expected_mc_year;
};

class ProblemConstructionTest: public testing::TestWithParam<ProblemNameCase> {

};

TEST_F(ProblemConstructionTest, ExtractMCYear1FromName) {
  std::string file_name = "problem-1-37-20220214-124051.mps";

  auto [year, week] = McYearAndWeek(file_name);
  EXPECT_EQ(year, 1);
  EXPECT_EQ(week, 37);
}

ProblemNameCase cases[] = {
    {"problem-1-1-20220214-124051.mps", 1},
    {"problem-2-1-20220214-124051.mps", 2},
    {"problem-1-2-20220214-124051.mps",1},
    {"garbage-3-1-20220214-124051.mps", 3}
};
TEST_P(ProblemConstructionTest, ExtractSeveralFileNameCase) {
  auto const& input = GetParam();
  EXPECT_EQ(McYearAndWeek(input.problem_name).first, input.expected_mc_year);
}
INSTANTIATE_TEST_SUITE_P(BulkTest, ProblemConstructionTest,
                         testing::ValuesIn(cases));

TEST_F(ProblemConstructionTest, ExtractMCYearFromPath) {
  std::filesystem::path path = std::filesystem::path("path") / "To" / "inner-dir" / "problem-2-10-20220214-124051.mps";

  auto [year, week] = McYearAndWeek(path);
  EXPECT_EQ(year, 2);
  EXPECT_EQ(week, 10);
}


TEST_F(ProblemConstructionTest, McYearAvailableInProblem) {
  auto solver = std::make_shared<NOOPSolver>();
  auto problem = std::make_shared<Problem>(solver) ;

  std::filesystem::path file_name("Path/To/inner-dir/problem-3-45-20220214-124051.mps");
  problem->read_prob_mps(file_name);
  EXPECT_EQ(problem->McYear(), 3);
  EXPECT_EQ(problem->Week(), 45);
}