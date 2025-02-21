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
        options_.MASTER_NAME = (tmp_dir_ / "master.mps"s).string();
    }

    void createStructureFile(const std::vector<std::tuple<std::string, std::string, int> > &entries) {
        std::ofstream structure_file(options_.STRUCTURE_FILE);
        for (const auto &[mps, var, idx]: entries) {
            structure_file << fmt::format("{3}/{0} {1} {2}\n", mps, var, idx, tmp_dir_.string());
        }
        structure_file.close();
    }

    std::filesystem::path tmp_dir_;
    MergeMPSOptions options_;
    std::shared_ptr<Xpansion::Test::LoggerNOOPStub> logger_;
    std::shared_ptr<Xpansion::Test::InMemoryWriter> writer_;
};

TEST(MergeMPS, empty_input_ok) {
    auto tmpDir = CreateRandomSubDir(std::filesystem::temp_directory_path());
    std::ofstream structure_file(tmpDir / "structure_file.txt"s);

    MergeMPSOptions options;
    options.SOLVER_NAME = "COIN";
    options.STRUCTURE_FILE = (tmpDir / "structure_file.txt"s).string();
    auto logger = std::make_shared<Xpansion::Test::LoggerNOOPStub>();
    auto writer = std::make_shared<Xpansion::Test::InMemoryWriter>();
    MergeMPS mergeMPS(options, logger, writer);
    mergeMPS.launch();
    auto lastSolution = writer->solution_data_;
    EXPECT_EQ(lastSolution.problem_status, "ERROR");
}

TEST_F(MergeMPSTest, one_master_Problem_ok) {
    createMasterProblem();
    createStructureFile({{"master.mps", "X1", 0}, {"master.mps", "X2", 1}});
    MergeMPS mergeMPS(options_, logger_, writer_);
    mergeMPS.launch();
    auto lastSolution = writer_->solution_data_;
    EXPECT_EQ(lastSolution.problem_status, "OPTIMAL");
    //log_merged_mps exists
    EXPECT_TRUE(std::filesystem::exists(tmp_dir_ / "log_merged.mps"s));
    EXPECT_TRUE(std::filesystem::exists(tmp_dir_ / "log_merged.lp"s));
    //Print the merged problem
    std::ifstream merged_mps(tmp_dir_ / "log_merged.mps"s);
    std::string line;
    while (std::getline(merged_mps, line)) {
        std::cout << line << std::endl;
    }
    merged_mps.close();
    SolverFactory factory;
    auto merged = factory.create_solver("CBC");
    merged->read_prob_lp(tmp_dir_ / "log_merged.lp"s);
    auto master = factory.create_solver("CBC");
    master->read_prob_mps(tmp_dir_ / "master.mps"s);
    //Merged and master problems are identical
    EXPECT_EQ(merged->get_ncols(), master->get_ncols());
    EXPECT_EQ(merged->get_nrows(), master->get_nrows());
    EXPECT_EQ(merged->get_nelems(), master->get_nelems());
    //Merged and master problems have the same objective function
    DblVector obj_merged(merged->get_ncols());
    DblVector obj_master(master->get_ncols());
    merged->get_obj(obj_merged.data(), 0, merged->get_ncols() - 1);
    master->get_obj(obj_master.data(), 0, master->get_ncols() - 1);
    for (int i = 0; i < merged->get_ncols(); ++i) {
        EXPECT_DOUBLE_EQ(obj_merged[i], obj_master[i]);
    }
    //BOth have the same solution
    DblVector sol_merged(merged->get_ncols());
    DblVector sol_master(master->get_ncols());
    merged->get_lp_sol(sol_merged.data(), nullptr, nullptr);
    master->get_lp_sol(sol_master.data(), nullptr, nullptr);
    for (int i = 0; i < merged->get_ncols(); ++i) {
        EXPECT_DOUBLE_EQ(sol_merged[i], sol_master[i]);
    }
}

//TEst with 2 problems the result is a problem with variables from both problems
TEST_F(MergeMPSTest, two_problems_ok) {
    createMasterProblem();
    createStructureFile({
        {"master.mps", "X1", 0}, {"master.mps", "X2", 1},
        {"slave1.mps", "Y1", 0}, {"slave1.mps", "Y2", 1}
    });
    std::ofstream slave1(tmp_dir_ / "slave1.mps");
    slave1 << R"(NAME          SLAVE1  FREE
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
    slave1.close();
    options_.weights[(tmp_dir_ / "slave1.mps").string()] = 1;
    MergeMPS mergeMPS(options_, logger_, writer_);
    mergeMPS.launch();
    auto lastSolution = writer_->solution_data_;
    EXPECT_EQ(lastSolution.problem_status, "OPTIMAL");
    //log_merged_mps exists
    EXPECT_TRUE(std::filesystem::exists(tmp_dir_ / "log_merged.mps"s));
    EXPECT_TRUE(std::filesystem::exists(tmp_dir_ / "log_merged.lp"s));
    SolverFactory factory;
    auto merged = factory.create_solver("CBC");
    merged->read_prob_mps(tmp_dir_ / "log_merged.lp"s);
    auto master = factory.create_solver("CBC");
    master->read_prob_mps(tmp_dir_ / "master.mps"s);
    auto slave = factory.create_solver("CBC");
    slave->read_prob_mps(tmp_dir_ / "slave1.mps"s);
    //Merged has master + slave number of constraints
    EXPECT_EQ(merged->get_nrows(), master->get_nrows() + slave->get_nrows());
    //Merged has master + slave number of variables
    EXPECT_EQ(merged->get_ncols(), master->get_ncols() + slave->get_ncols());
    //Merged obj is the combination of master and slave
    DblVector obj_merged(merged->get_ncols());
    DblVector obj_master(master->get_ncols());
    DblVector obj_slave(slave->get_ncols());
    merged->get_obj(obj_merged.data(), 0, merged->get_ncols() - 1);
    master->get_obj(obj_master.data(), 0, master->get_ncols() - 1);
    slave->get_obj(obj_slave.data(), 0, slave->get_ncols() - 1);
    for (int i = 0; i < master->get_ncols(); ++i) {
        EXPECT_DOUBLE_EQ(obj_merged[i], obj_master[i]);
    }
    for (int i = 0; i < slave->get_ncols(); ++i) {
        EXPECT_DOUBLE_EQ(obj_merged[i + master->get_ncols()], obj_slave[i]);
    }
    //Merged has the same solution as master and slave
    DblVector sol_merged(merged->get_ncols());
    DblVector sol_master(master->get_ncols());
    DblVector sol_slave(slave->get_ncols());
    merged->get_lp_sol(sol_merged.data(), nullptr, nullptr);
    master->get_lp_sol(sol_master.data(), nullptr, nullptr);
    slave->get_lp_sol(sol_slave.data(), nullptr, nullptr);
    for (int i = 0; i < master->get_ncols(); ++i) {
        EXPECT_DOUBLE_EQ(sol_merged[i], sol_master[i]);
    }
    for (int i = 0; i < slave->get_ncols(); ++i) {
        EXPECT_DOUBLE_EQ(sol_merged[i + master->get_ncols()], sol_slave[i]);
    }
}
