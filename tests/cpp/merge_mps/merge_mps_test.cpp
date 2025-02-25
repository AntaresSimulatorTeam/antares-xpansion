#include <fmt/format.h>
#include <gtest/gtest.h>

#include "LoggerStub.h"
#include "RandomDirGenerator.h"
#include "InMemoryWriter.h"
#include "antares-xpansion/benders/merge_mps/MergeMPS.h"

size_t StandardLp::appendCNT = 0;

using namespace std::string_literals;

class MergeMPSTest : public ::testing::Test {
protected:
    void SetUp() override {
        tmp_dir_ = CreateRandomSubDir(std::filesystem::temp_directory_path());
        logger_ = std::make_shared<Xpansion::Test::LoggerNOOPStub>();
        writer_ = std::make_shared<Xpansion::Test::InMemoryWriter>();

        options_.SOLVER_NAME = "COIN";
        options_.STRUCTURE_FILE = (tmp_dir_ / "structure_file.txt").string();
        options_.OUTPUTROOT = tmp_dir_.string();
    }

    void TearDown() override {
    }

    /**
     * Create the following problem :
     * Minimize
     * Obj: 3X1 + 4X2
     * 2x1 + x2 <= 8
     * x1 + 2x2 >= 3
     * 0 <=X1 <= 4
     * 0 <=X2 <= 4
     */
    void createMasterProblem() {
        std::ofstream master_problem(tmp_dir_ / "master.mps"s);
        master_problem << R"(NAME          MASTER  FREE
ROWS
 N  OBJROW
 L  C1
 G  C2
COLUMNS
    X1        OBJROW       3.0   C1        2.0
    X1        C2        1.0
    X2        OBJROW       4.0   C1        1.0
    X2        C2        2.0
RHS
    RHS      C1        8.0
    RHS      C2        3.0
BOUNDS
 UP BOUND      X1        4.0
 UP BOUND      X2        4.0
ENDATA)";
        master_problem.close();
        options_.MASTER_NAME = "master.mps";
    }

    void createStructureFile(const std::vector<std::tuple<std::string, std::string, int> > &entries) {
        std::ofstream structure_file(options_.STRUCTURE_FILE);
        for (const auto &[mps, var, idx]: entries) {
            structure_file << fmt::format("{0} {1} {2}\n", mps, var, idx);
        }
        structure_file.close();
    }

    std::filesystem::path tmp_dir_;
    MergeMPSOptions options_;
    std::shared_ptr<Xpansion::Test::LoggerNOOPStub> logger_;
    std::shared_ptr<Xpansion::Test::InMemoryWriter> writer_;
};

TEST_F(MergeMPSTest, empty_input_ok) {
    std::ofstream structure_file(tmp_dir_ / "structure_file.txt"s);

    MergeMPSOptions options;
    options.SOLVER_NAME = "COIN";
    options.STRUCTURE_FILE = (tmp_dir_ / "structure_file.txt"s).string();
    auto logger = std::make_shared<Xpansion::Test::LoggerNOOPStub>();
    auto writer = std::make_shared<Xpansion::Test::InMemoryWriter>();
    std::filesystem::current_path(tmp_dir_);
    MergeMPS mergeMPS(options, logger, writer);
    mergeMPS.launch();
    auto lastSolution = writer->solution_data_;
    EXPECT_EQ(lastSolution.problem_status, "ERROR");
}

TEST_F(MergeMPSTest, one_master_Problem_ok) {
    createMasterProblem();
    createStructureFile({{"master.mps", "X1", 0}, {"master.mps", "X2", 1}});
    std::filesystem::current_path(tmp_dir_);
    MergeMPS mergeMPS(options_, logger_, writer_);
    mergeMPS.launch();

    auto lastSolution = writer_->solution_data_;
    EXPECT_EQ(lastSolution.problem_status, "OPTIMAL");

    EXPECT_TRUE(std::filesystem::exists(tmp_dir_ / "log_merged.mps"s));
    EXPECT_TRUE(std::filesystem::exists(tmp_dir_ / "log_merged.lp"s));

    SolverFactory factory;
    auto merged = factory.create_solver("CBC");
    merged->read_prob_mps(tmp_dir_ / "log_merged.mps"s);
    auto master = factory.create_solver("CBC");
    master->read_prob_mps(tmp_dir_ / "master.mps"s);

    EXPECT_TRUE(*merged.get() == *master.get());
}

//Test with 2 problems not sharing variables
TEST_F(MergeMPSTest, two_disconected_problems_ok) {
    createMasterProblem();
    createStructureFile({
        {"master.mps", "X1", 0}, {"master.mps", "X2", 1},
        {"satellite1.mps", "Y1", 0}, {"satellite1.mps", "Y2", 1}
    });
    std::ofstream satellite1(tmp_dir_ / "satellite1.mps");
    satellite1 << R"(NAME          satellite1  FREE
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
    satellite1.close();

    //Required to avoid missing contribution to Obj
    options_.weights["satellite1.mps"] = 1;

    std::filesystem::current_path(tmp_dir_);
    MergeMPS mergeMPS(options_, logger_, writer_);
    mergeMPS.launch();

    auto lastSolution = writer_->solution_data_;
    EXPECT_EQ(lastSolution.problem_status, "OPTIMAL");

    EXPECT_TRUE(std::filesystem::exists(tmp_dir_ / "log_merged.mps"s));
    EXPECT_TRUE(std::filesystem::exists(tmp_dir_ / "log_merged.lp"s));

    SolverFactory factory;
    auto merged = factory.create_solver("CBC");
    merged->read_prob_mps(tmp_dir_ / "log_merged.mps"s);

    auto master = factory.create_solver("CBC");
    master->read_prob_mps(tmp_dir_ / "master.mps"s);

    auto satellite = factory.create_solver("CBC");
    satellite->read_prob_mps(tmp_dir_ / "satellite1.mps"s);

    //Merged has master + satellite number of constraints
    EXPECT_EQ(merged->get_nrows(), master->get_nrows() + satellite->get_nrows());
    //Merged has master + satellite number of variables
    EXPECT_EQ(merged->get_ncols(), master->get_ncols() + satellite->get_ncols());
    //Merged obj is the combination of master and satellite
    DblVector obj_merged(merged->get_ncols());
    DblVector obj_master(master->get_ncols());
    DblVector obj_satellite(satellite->get_ncols());
    merged->get_obj(obj_merged.data(), 0, merged->get_ncols() - 1);
    master->get_obj(obj_master.data(), 0, master->get_ncols() - 1);
    satellite->get_obj(obj_satellite.data(), 0, satellite->get_ncols() - 1);
    for (int i = 0; i < master->get_ncols(); ++i) {
        EXPECT_DOUBLE_EQ(obj_merged[i], obj_master[i]);
    }
    for (int i = 0; i < satellite->get_ncols(); ++i) {
        EXPECT_DOUBLE_EQ(obj_merged[i + master->get_ncols()], obj_satellite[i]);
    }
    //Merged has the same solution as master and satellite
    DblVector sol_merged(merged->get_ncols());
    DblVector sol_master(master->get_ncols());
    DblVector sol_satellite(satellite->get_ncols());
    merged->get_lp_sol(sol_merged.data(), nullptr, nullptr);
    master->get_lp_sol(sol_master.data(), nullptr, nullptr);
    satellite->get_lp_sol(sol_satellite.data(), nullptr, nullptr);
    for (int i = 0; i < master->get_ncols(); ++i) {
        EXPECT_DOUBLE_EQ(sol_merged[i], sol_master[i]);
    }
    for (int i = 0; i < satellite->get_ncols(); ++i) {
        EXPECT_DOUBLE_EQ(sol_merged[i + master->get_ncols()], sol_satellite[i]);
    }
}
