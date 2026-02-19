#include "MergeMpsFixture.h"

TEST_F(MergeMPSTest, merged_problems_adds_one_coupling_constraint)
{
    createMasterProblem();
    createSatelliteProblem();
    createStructureFile(
      {{"master.mps", "X1", 0}, {"master.mps", "X2", 1}, {"satellite.mps", "X1", 1}});

    MergeMPS mergeMPS(options_, logger_, writer_);
    mergeMPS.launch();

    auto lastSolution = writer_->solution_data_;
    EXPECT_EQ(lastSolution.problem_status, "OPTIMAL");

    EXPECT_TRUE(std::filesystem::exists(tmp_dir_ / "log_merged.mps"s));
    EXPECT_TRUE(!std::filesystem::exists(tmp_dir_ / "log_merged.lp"s));

    SolverFactory factory;
    auto merged = factory.create_solver("CBC");
    merged->read_prob_mps(tmp_dir_ / "log_merged.mps"s);

    auto master = factory.create_solver("CBC");
    master->read_prob_mps(tmp_dir_ / "master.mps"s);

    auto satellite = factory.create_solver("CBC");
    satellite->read_prob_mps(tmp_dir_ / "satellite.mps"s);

    EXPECT_EQ(merged->get_nrows(),
              master->get_nrows() + satellite->get_nrows() + 1);
    EXPECT_EQ(merged->get_ncols(),
              master->get_ncols() + satellite->get_ncols());
    EXPECT_EQ(merged->get_nelems(),
              master->get_nelems() + satellite->get_nelems()
                + 2);
}

TEST_F(MergeMPSTest, coupling_constraints_with_variable_in_three_problems)
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
    X1        C1        0.5
RHS
    RHS      C1        5.0
BOUNDS
 UP BOUND      Z1        5.0
 UP BOUND      X1        10.0
ENDATA)";
    third_problem.close();
    options_.weights["third.mps"] = 1;

    createStructureFile({{"master.mps", "X1", 0},
                         {"master.mps", "X2", 1},
                         {"satellite.mps", "X1", 1},
                         {"third.mps", "X1", 1}});

    MergeMPS mergeMPS(options_, logger_, writer_);
    mergeMPS.launch();

    auto lastSolution = writer_->solution_data_;
    EXPECT_EQ(lastSolution.problem_status, "OPTIMAL");

    SolverFactory factory;
    auto merged = factory.create_solver("CBC");
    merged->read_prob_mps(tmp_dir_ / "log_merged.mps"s);

    auto master = factory.create_solver("CBC");
    master->read_prob_mps(tmp_dir_ / "master.mps"s);
    auto satellite = factory.create_solver("CBC");
    satellite->read_prob_mps(tmp_dir_ / "satellite.mps"s);
    auto third = factory.create_solver("CBC");
    third->read_prob_mps(tmp_dir_ / "third.mps"s);

    EXPECT_EQ(merged->get_nrows(),
              master->get_nrows() + satellite->get_nrows() + third->get_nrows() + 2);
}

