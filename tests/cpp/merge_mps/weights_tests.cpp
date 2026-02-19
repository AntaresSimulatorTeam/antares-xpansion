#include "MergeMpsFixture.h"

TEST_F(MergeMPSTest, merged_problems_objective_coeff_are_multiplied_by_factor)
{
    createMasterProblem();
    createSatelliteProblem();
    createStructureFile(
      {{"master.mps", "X1", 0}, {"master.mps", "X2", 1}, {"satellite.mps", "X1", 1}});

    const double factor = 3.;
    options_.weights["satellite.mps"] = factor;

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

    std::vector<double> obj_merged(merged->get_ncols());
    std::vector<double> obj_master(master->get_ncols());
    std::vector<double> obj_satellite(satellite->get_ncols());
    merged->get_obj(obj_merged.data(), 0, merged->get_ncols() - 1);
    master->get_obj(obj_master.data(), 0, master->get_ncols() - 1);
    satellite->get_obj(obj_satellite.data(), 0, satellite->get_ncols() - 1);

    for (int i = 0; i < master->get_ncols(); ++i)
    {
        EXPECT_DOUBLE_EQ(obj_merged[i], obj_master[i]);
    }
    for (int i = 0; i < satellite->get_ncols(); ++i)
    {
        EXPECT_DOUBLE_EQ(obj_merged[i + master->get_ncols()], factor * obj_satellite[i]);
    }
}

TEST_F(MergeMPSTest, merged_problems_with_zero_weight)
{
    createMasterProblem();
    createSatelliteProblem();
    createStructureFile(
      {{"master.mps", "X1", 0}, {"master.mps", "X2", 1}, {"satellite.mps", "X1", 1}});

    options_.weights["satellite.mps"] = 0.0;

    MergeMPS mergeMPS(options_, logger_, writer_);
    mergeMPS.launch();

    auto lastSolution = writer_->solution_data_;
    EXPECT_EQ(lastSolution.problem_status, "OPTIMAL");

    SolverFactory factory;
    auto merged = factory.create_solver("CBC");
    merged->read_prob_mps(tmp_dir_ / "log_merged.mps"s);

    std::vector<double> obj_merged(merged->get_ncols());
    merged->get_obj(obj_merged.data(), 0, merged->get_ncols() - 1);

    auto master = factory.create_solver("CBC");
    master->read_prob_mps(tmp_dir_ / "master.mps"s);

    for (int i = master->get_ncols(); i < merged->get_ncols(); ++i)
    {
        EXPECT_DOUBLE_EQ(obj_merged[i], 0.0);
    }
}

TEST_F(MergeMPSTest, negative_weight_multiplier)
{
    createMasterProblem();
    createSatelliteProblem();
    createStructureFile(
      {{"master.mps", "X1", 0}, {"master.mps", "X2", 1}, {"satellite.mps", "X1", 1}});

    const double factor = -2.5;
    options_.weights["satellite.mps"] = factor;

    MergeMPS mergeMPS(options_, logger_, writer_);
    mergeMPS.launch();

    SolverFactory factory;
    auto merged = factory.create_solver("CBC");
    merged->read_prob_mps(tmp_dir_ / "log_merged.mps"s);

    auto master = factory.create_solver("CBC");
    master->read_prob_mps(tmp_dir_ / "master.mps"s);

    auto satellite = factory.create_solver("CBC");
    satellite->read_prob_mps(tmp_dir_ / "satellite.mps"s);

    std::vector<double> obj_merged(merged->get_ncols());
    std::vector<double> obj_satellite(satellite->get_ncols());
    merged->get_obj(obj_merged.data(), 0, merged->get_ncols() - 1);
    satellite->get_obj(obj_satellite.data(), 0, satellite->get_ncols() - 1);

    for (int i = 0; i < satellite->get_ncols(); ++i)
    {
        EXPECT_DOUBLE_EQ(obj_merged[i + master->get_ncols()], factor * obj_satellite[i]);
    }
}

TEST_F(MergeMPSTest, uniform_slave_weight_distribution)
{
    createMasterProblem();
    createSatelliteProblem();

    std::ofstream satellite2_problem(tmp_dir_ / "satellite2.mps"s);
    satellite2_problem << R"(NAME       SATELLITE2  FREE
ROWS
 N  OBJROW
 L  C1
COLUMNS
    Y2        OBJROW    5.0
    Y2        C1        1.0
RHS
    RHS      C1        10.0
BOUNDS
 UP BOUND      Y2        10.0
ENDATA)";
    satellite2_problem.close();

    createStructureFile({{"master.mps", "X1", 0},
                         {"master.mps", "X2", 1},
                         {"satellite.mps", "Y1", 0},
                         {"satellite2.mps", "Y2", 0}});

    options_.SLAVE_WEIGHT = "UNIFORM";

    MergeMPS mergeMPS(options_, logger_, writer_);
    mergeMPS.launch();

    auto lastSolution = writer_->solution_data_;
    EXPECT_EQ(lastSolution.problem_status, "OPTIMAL");

    SolverFactory factory;
    auto merged = factory.create_solver("CBC");
    merged->read_prob_mps(tmp_dir_ / "log_merged.mps"s);

    std::vector<double> obj_merged(merged->get_ncols());
    merged->get_obj(obj_merged.data(), 0, merged->get_ncols() - 1);

    auto satellite = factory.create_solver("CBC");
    satellite->read_prob_mps(tmp_dir_ / "satellite.mps"s);
    std::vector<double> obj_satellite(satellite->get_ncols());
    satellite->get_obj(obj_satellite.data(), 0, satellite->get_ncols() - 1);

    auto master = factory.create_solver("CBC");
    master->read_prob_mps(tmp_dir_ / "master.mps"s);

    const double expected_weight = 1.0 / 2.0;
    for (int i = 0; i < satellite->get_ncols(); ++i)
    {
        EXPECT_DOUBLE_EQ(obj_merged[i + master->get_ncols()],
                        expected_weight * obj_satellite[i]);
    }
}

TEST_F(MergeMPSTest, constant_slave_weight_mode)
{
    createMasterProblem();

    std::ofstream satellite_problem(tmp_dir_ / "satellite.mps"s);
    satellite_problem << R"(NAME       SATELLITE  FREE
ROWS
 N  OBJROW
 L  C1
 G  C2
COLUMNS
    Y1        OBJROW    1.0
    Y1        C1        3.0
    Y1        C2        9.0
    X1        C1        2.0
    X1        C2        3.0
RHS
    RHS      C1        7.0
    RHS      C2        5.0
BOUNDS
 UP BOUND      Y1        10.0
 UP BOUND      X1        10.0
ENDATA)";
    satellite_problem.close();

    createStructureFile(
      {{"master.mps", "X1", 0}, {"master.mps", "X2", 1}, {"satellite.mps", "X1", 1}});

    options_.SLAVE_WEIGHT = "CONSTANT";
    options_.SLAVE_WEIGHT_VALUE = 52;

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

    std::vector<double> obj_merged(merged->get_ncols());
    std::vector<double> obj_satellite(satellite->get_ncols());
    merged->get_obj(obj_merged.data(), 0, merged->get_ncols() - 1);
    satellite->get_obj(obj_satellite.data(), 0, satellite->get_ncols() - 1);

    const double expected_weight = 1.0 / 52.0;
    for (int i = 0; i < satellite->get_ncols(); ++i)
    {
        EXPECT_DOUBLE_EQ(obj_merged[i + master->get_ncols()],
                        expected_weight * obj_satellite[i]);
    }
}

TEST_F(MergeMPSTest, missing_weight_warning_zero_contribution)
{
    createMasterProblem();
    createSatelliteProblem();
    createStructureFile(
      {{"master.mps", "X1", 0}, {"master.mps", "X2", 1}, {"satellite.mps", "X1", 1}});

    options_.weights.erase("satellite.mps");

    MergeMPS mergeMPS(options_, logger_, writer_);
    mergeMPS.launch();

    auto lastSolution = writer_->solution_data_;
    EXPECT_EQ(lastSolution.problem_status, "OPTIMAL");

    SolverFactory factory;
    auto merged = factory.create_solver("CBC");
    merged->read_prob_mps(tmp_dir_ / "log_merged.mps"s);

    std::vector<double> obj_merged(merged->get_ncols());
    merged->get_obj(obj_merged.data(), 0, merged->get_ncols() - 1);

    auto master = factory.create_solver("CBC");
    master->read_prob_mps(tmp_dir_ / "master.mps"s);

    for (int i = master->get_ncols(); i < merged->get_ncols(); ++i)
    {
        EXPECT_DOUBLE_EQ(obj_merged[i], 0.0);
    }
}

TEST_F(MergeMPSTest, weight_near_one_skips_multiplication)
{
    createMasterProblem();
    createSatelliteProblem();
    createStructureFile(
      {{"master.mps", "X1", 0}, {"master.mps", "X2", 1}, {"satellite.mps", "X1", 1}});

    options_.weights["satellite.mps"] = 1.0 + 1e-7;

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

    std::vector<double> obj_merged(merged->get_ncols());
    std::vector<double> obj_satellite(satellite->get_ncols());
    merged->get_obj(obj_merged.data(), 0, merged->get_ncols() - 1);
    satellite->get_obj(obj_satellite.data(), 0, satellite->get_ncols() - 1);

    for (int i = 0; i < satellite->get_ncols(); ++i)
    {
        EXPECT_DOUBLE_EQ(obj_merged[i + master->get_ncols()], obj_satellite[i]);
    }
}

