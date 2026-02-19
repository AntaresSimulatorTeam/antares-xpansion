#include "MergeMpsFixture.h"

TEST_F(MergeMPSTest, merged_problems_result)
{
    createMasterProblem();
    createSatelliteProblem();
    createStructureFile(
      {{"master.mps", "X1", 0}, {"master.mps", "X2", 1}, {"satellite.mps", "X1", 1}});

    MergeMPS mergeMPS(options_, logger_, writer_);
    mergeMPS.launch();

    auto lastSolution = writer_->solution_data_;
    EXPECT_EQ(lastSolution.problem_status, "OPTIMAL");

    EXPECT_DOUBLE_EQ(lastSolution.solution.investment_cost, 6.);
    EXPECT_DOUBLE_EQ(lastSolution.solution.operational_cost, 5. / 9);
    EXPECT_DOUBLE_EQ(lastSolution.solution.overall_cost, 6. + 5. / 9);

    EXPECT_DOUBLE_EQ(lastSolution.solution.candidates[0].invest, 0.0);
    EXPECT_DOUBLE_EQ(lastSolution.solution.candidates[1].invest, 3. / 2);
}

TEST_F(MergeMPSTest, merged_with_three_problems)
{
    createMasterProblem();
    createSatelliteProblem();

    std::ofstream third_problem(tmp_dir_ / "third.mps"s);
    third_problem << R"(NAME       THIRD  FREE
ROWS
 N  OBJROW
 L  C1
COLUMNS
    Z1        OBJROW    2.0
    Z1        C1        1.0
RHS
    RHS      C1        5.0
BOUNDS
 UP BOUND      Z1        5.0
ENDATA)";
    third_problem.close();
    options_.weights["third.mps"] = 1;

    createStructureFile({{"master.mps", "X1", 0},
                         {"master.mps", "X2", 1},
                         {"satellite.mps", "X1", 1},
                         {"third.mps", "Z1", 0}});

    MergeMPS mergeMPS(options_, logger_, writer_);
    mergeMPS.launch();

    auto lastSolution = writer_->solution_data_;
    EXPECT_EQ(lastSolution.problem_status, "OPTIMAL");

    SolverFactory factory;
    auto merged = factory.create_solver("CBC");
    merged->read_prob_mps(tmp_dir_ / "log_merged.mps"s);

    auto master = factory.create_solver("CBC");
    master->read_prob_mps(tmp_dir_ / "master.mps"s);

    EXPECT_GT(merged->get_ncols(), master->get_ncols());
}

