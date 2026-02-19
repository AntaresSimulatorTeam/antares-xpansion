#include "MergeMpsFixture.h"

TEST_F(MergeMPSTest, output_lp_format_not_generated_by_default)
{
    createMasterProblem();
    createStructureFile({{"master.mps", "X1", 0}, {"master.mps", "X2", 1}});

    MergeMPS mergeMPS(options_, logger_, writer_);
    mergeMPS.launch();

    EXPECT_FALSE(std::filesystem::exists(tmp_dir_ / "log_merged.lp"s));
    EXPECT_TRUE(std::filesystem::exists(tmp_dir_ / "log_merged.mps"s));
}

TEST_F(MergeMPSTest, solver_name_coin_mapped_to_cbc)
{
    createMasterProblem();
    createStructureFile({{"master.mps", "X1", 0}, {"master.mps", "X2", 1}});

    options_.SOLVER_NAME = "COIN";

    MergeMPS mergeMPS(options_, logger_, writer_);
    mergeMPS.launch();

    auto lastSolution = writer_->solution_data_;
    EXPECT_EQ(lastSolution.problem_status, "OPTIMAL");
}

