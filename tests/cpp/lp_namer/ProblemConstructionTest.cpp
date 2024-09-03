//
// Created by marechaljas on 02/05/2022.
//

#include <gtest/gtest.h>
#include <filesystem>
#include "ProblemNameParser.h"
#include "Problem.h"
#include "multisolver_interface/SolverFactory.h"
#include "NOOPSolver.h"

struct ProblemNameCase {
  std::string problem_name;
  unsigned int expected_mc_year;
};

class ProblemConstructionTest: public testing::TestWithParam<ProblemNameCase> {

};

TEST_F(ProblemConstructionTest, ExtractMCYear1FromName) {
  std::string file_name = "problem-1-1-20220214-124051.mps";

  unsigned int res = MCYear(file_name);
  EXPECT_EQ(res, 1);
}

ProblemNameCase cases[] = {
    {"problem-1-1-20220214-124051.mps", 1},
    {"problem-2-1-20220214-124051.mps", 2},
    {"problem-1-2-20220214-124051.mps",1},
    {"garbage-3-1-20220214-124051.mps", 3}
};
TEST_P(ProblemConstructionTest, ExtractSeveralFileNameCase) {
  auto const& input = GetParam();
  EXPECT_EQ(MCYear(input.problem_name), input.expected_mc_year);
}
INSTANTIATE_TEST_SUITE_P(BulkTest, ProblemConstructionTest,
                         testing::ValuesIn(cases));

TEST_F(ProblemConstructionTest, ExtractMCYearFromPath) {
  std::filesystem::path path = std::filesystem::path("path") / "To" / "inner-dir" / "problem-2-1-20220214-124051.mps";

  EXPECT_EQ(MCYear(path), 2);
}


TEST_F(ProblemConstructionTest, McYearAvailableInProblem) {
  auto solver = std::make_shared<NOOPSolver>();
  auto problem = std::make_shared<Problem>(solver) ;

  std::filesystem::path file_name("Path/To/inner-dir/problem-3-45-20220214-124051.mps");
  problem->read_prob_mps(file_name);
  EXPECT_EQ(problem->McYear(), 3);
}