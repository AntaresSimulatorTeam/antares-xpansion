#include "MergeMpsFixture.h"

TEST_F(MergeMPSTest, merged_problems_lhs_does_not_change)
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

    auto [mstart_merged, mclind_merged, dmatval_merged, merge_ret] = get_rows(merged.get());
    auto [mstart_master, mclind_master, dmatval_master, master_ret] = get_rows(master.get());
    auto [mstart_satellite, mclind_satellite, dmatval_satellite, satellite_ret] = get_rows(
      satellite.get());

    for (int i = 0; i < master->get_nelems(); ++i)
    {
        EXPECT_DOUBLE_EQ(dmatval_merged[i], dmatval_master[i]);
    }
    for (int i = 0; i < satellite->get_nelems(); ++i)
    {
        EXPECT_DOUBLE_EQ(dmatval_merged[i + master->get_nelems()], dmatval_satellite[i]);
    }
}

TEST_F(MergeMPSTest, merged_problems_lb_does_not_change)
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

    std::vector<double> lb_merged(merged->get_ncols());
    std::vector<double> lb_master(master->get_ncols());
    std::vector<double> lb_satellite(satellite->get_ncols());
    merged->get_lb(lb_merged.data(), 0, merged->get_ncols() - 1);
    master->get_lb(lb_master.data(), 0, master->get_ncols() - 1);
    satellite->get_lb(lb_satellite.data(), 0, satellite->get_ncols() - 1);

    for (int i = 0; i < master->get_ncols(); ++i)
    {
        EXPECT_DOUBLE_EQ(lb_merged[i], lb_master[i]);
    }
    for (int i = 0; i < satellite->get_ncols(); ++i)
    {
        EXPECT_DOUBLE_EQ(lb_merged[i + master->get_ncols()], lb_satellite[i]);
    }
}

TEST_F(MergeMPSTest, merged_problems_ub_does_not_change)
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

    std::vector<double> ub_merged(merged->get_ncols());
    std::vector<double> ub_master(master->get_ncols());
    std::vector<double> ub_satellite(satellite->get_ncols());
    merged->get_ub(ub_merged.data(), 0, merged->get_ncols() - 1);
    master->get_ub(ub_master.data(), 0, master->get_ncols() - 1);
    satellite->get_ub(ub_satellite.data(), 0, satellite->get_ncols() - 1);

    for (int i = 0; i < master->get_ncols(); ++i)
    {
        EXPECT_DOUBLE_EQ(ub_merged[i], ub_master[i]);
    }
    for (int i = 0; i < satellite->get_ncols(); ++i)
    {
        EXPECT_DOUBLE_EQ(ub_merged[i + master->get_ncols()], ub_satellite[i]);
    }
}

TEST_F(MergeMPSTest, merged_problems_rhs_values_does_not_change)
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

    std::vector<double> rhs_merged(merged->get_nrows());
    std::vector<double> rhs_master(master->get_nrows());
    std::vector<double> rhs_satellite(satellite->get_nrows());
    merged->get_rhs(rhs_merged.data(), 0, merged->get_nrows() - 1);
    master->get_rhs(rhs_master.data(), 0, master->get_nrows() - 1);
    satellite->get_rhs(rhs_satellite.data(), 0, satellite->get_nrows() - 1);

    for (int i = 0; i < master->get_nrows(); ++i)
    {
        EXPECT_DOUBLE_EQ(rhs_merged[i], rhs_master[i]);
    }
    for (int i = 0; i < satellite->get_nrows(); ++i)
    {
        EXPECT_DOUBLE_EQ(rhs_merged[i + master->get_nrows()], rhs_satellite[i]);
    }
}

TEST_F(MergeMPSTest, merged_problems_rhs_types_does_not_change)
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

    std::vector<char> order_merged(merged->get_nrows());
    std::vector<char> order_master(master->get_nrows());
    std::vector<char> order_satellite(satellite->get_nrows());
    merged->get_row_type(order_merged.data(), 0, merged->get_nrows() - 1);
    master->get_row_type(order_master.data(), 0, master->get_nrows() - 1);
    satellite->get_row_type(order_satellite.data(), 0, satellite->get_nrows() - 1);

    for (int i = 0; i < master->get_nrows(); ++i)
    {
        EXPECT_EQ(order_merged[i], order_master[i]);
    }
    for (int i = 0; i < satellite->get_nrows(); ++i)
    {
        EXPECT_EQ(order_merged[i + master->get_nrows()], order_satellite[i]);
    }
}

