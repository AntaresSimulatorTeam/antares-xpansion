#include "MergeMpsFixture.h"

TEST_F(MergeMPSTest, empty_input_ok)
{
    std::ofstream structure_file(tmp_dir_ / "structure_file.txt"s);
    structure_file.close();

    MergeMPS mergeMPS(options_, logger_, writer_);
    mergeMPS.launch();
    const auto& lastSolution = writer_->solution_data_;
    EXPECT_EQ(lastSolution.problem_status, "ERROR");
}

TEST_F(MergeMPSTest, only_master_identical_to_merged)
{
    createMasterProblem();
    createStructureFile({{"master.mps", "X1", 0}, {"master.mps", "X2", 1}});
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

    EXPECT_EQ(*merged, *master);
}

TEST_F(MergeMPSTest, two_disconnected_problems_merged_is_concatenation_of_both)
{
    createMasterProblem();
    createStructureFile({{"master.mps", "X1", 0},
                         {"master.mps", "X2", 1},
                         {"disconnected.mps", "Y1", 0},
                         {"disconnected.mps", "Y2", 1}});
    std::ofstream disconnected(tmp_dir_ / "disconnected.mps");
    disconnected << R"(NAME          disconnected  FREE
ROWS
    N  OBJROW
    L  C10
    G  C20
COLUMNS
    Y1        OBJROW    1.0
    Y1        C10       1.0
    Y1        C20       1.0
    Y2        OBJROW    1.0
    Y2        C10        1.0
    Y2        C20        2.0
RHS
    RHS      C10        100.0
    RHS      C20        50.0
BOUNDS
    UP BOUND      Y1        50.0
    UP BOUND      Y2        50.0
ENDATA)";
    disconnected.close();

    options_.weights["disconnected.mps"] = 1;

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
    satellite->read_prob_mps(tmp_dir_ / "disconnected.mps"s);

    EXPECT_EQ(merged->get_nrows(), master->get_nrows() + satellite->get_nrows());
    EXPECT_EQ(merged->get_ncols(), master->get_ncols() + satellite->get_ncols());
    EXPECT_EQ(merged->get_nelems(), master->get_nelems() + satellite->get_nelems());
}

