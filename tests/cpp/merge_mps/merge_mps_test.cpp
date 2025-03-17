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
        previous_path = std::filesystem::current_path();
        std::filesystem::current_path(tmp_dir_);
        logger_ = std::make_shared<Xpansion::Test::LoggerNOOPStub>();
        writer_ = std::make_shared<Xpansion::Test::InMemoryWriter>();

        options_.SOLVER_NAME = "COIN";
        options_.STRUCTURE_FILE = (tmp_dir_ / "structure_file.txt").string();
        options_.OUTPUTROOT = tmp_dir_.string();
    }

    void TearDown() override {
        std::filesystem::current_path(previous_path);
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

    std::filesystem::path previous_path;
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

    MergeMPS mergeMPS(options, logger_, writer_);
    mergeMPS.launch();
    const auto &lastSolution = writer_->solution_data_;
    EXPECT_EQ(lastSolution.problem_status, "ERROR");
}

/**
 * Result and master should be identical
 */
TEST_F(MergeMPSTest, only_master_identical_to_merged) {
    createMasterProblem();
    createStructureFile({{"master.mps", "X1", 0}, {"master.mps", "X2", 1}});
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

    EXPECT_EQ(*merged, *master);
}

auto get_rows(const SolverAbstract *solver) {
    std::vector<int> mstart_merged(solver->get_nrows() + 1);
    std::vector<int> mclind_merged(solver->get_nelems());
    std::vector<double> dmatval_merged(solver->get_nelems());
    int merge_ret = 0;
    solver->get_rows(mstart_merged.data(), mclind_merged.data(), dmatval_merged.data(), solver->get_nelems(),
                     &merge_ret,
                     0,
                     solver->get_nrows() - 1);
    return std::tuple{mstart_merged, mclind_merged, dmatval_merged, merge_ret};
}

//Test with 2 problems not sharing variables
/**
 * Expectation :
\Problem name: ClpDefaultName

Minimize
obj: 3 x0 + 4 x1 + x2 + x3
Subject To
cons0:  2 x0 + x1 <= 8
cons1:  x0 + 2 x1 >= 3
cons2:  x2 + x3 <= 100
cons3:  x2 + 2 x3 >= 50
Bounds
0 <= x0 <= 4
0 <= x1 <= 4
0 <= x2 <= 50
0 <= x3 <= 50
End
*/
TEST_F(MergeMPSTest, two_disconected_problems_merged_is_concatenation_of_both) {
    createMasterProblem();
    createStructureFile({
        {"master.mps", "X1", 0}, {"master.mps", "X2", 1},
        {"satellite1.mps", "Y1", 0}, {"satellite1.mps", "Y2", 1}
    });
    /* Satellite :
     * Minimize Y1 + Y2
     * Subject To
     * C10: Y1 <= 100
     * C20: Y2 <= 50
     * Bounds
     * 0 <= Y1 <= 50
     * 0 <= Y2 <= 50
     */
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

    //Verify dimensions
    EXPECT_EQ(merged->get_nrows(), master->get_nrows() + satellite->get_nrows());
    EXPECT_EQ(merged->get_ncols(), master->get_ncols() + satellite->get_ncols());
    EXPECT_EQ(merged->get_nelems(), master->get_nelems() + satellite->get_nelems());

    //Verify objective
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

    //Verify lb
    DblVector lb_merged(merged->get_ncols());
    DblVector lb_master(master->get_ncols());
    DblVector lb_satellite(satellite->get_ncols());
    merged->get_lb(lb_merged.data(), 0, merged->get_ncols() - 1);
    master->get_lb(lb_master.data(), 0, master->get_ncols() - 1);
    satellite->get_lb(lb_satellite.data(), 0, satellite->get_ncols() - 1);
    for (int i = 0; i < master->get_ncols(); ++i) {
        EXPECT_DOUBLE_EQ(lb_merged[i], lb_master[i]);
    }
    for (int i = 0; i < satellite->get_ncols(); ++i) {
        EXPECT_DOUBLE_EQ(lb_merged[i + master->get_ncols()], lb_satellite[i]);
    }

    //verify ub
    DblVector ub_merged(merged->get_ncols());
    DblVector ub_master(master->get_ncols());
    DblVector ub_satellite(satellite->get_ncols());
    merged->get_ub(ub_merged.data(), 0, merged->get_ncols() - 1);
    master->get_ub(ub_master.data(), 0, master->get_ncols() - 1);
    satellite->get_ub(ub_satellite.data(), 0, satellite->get_ncols() - 1);
    for (int i = 0; i < master->get_ncols(); ++i) {
        EXPECT_DOUBLE_EQ(ub_merged[i], ub_master[i]);
    }
    for (int i = 0; i < satellite->get_ncols(); ++i) {
        EXPECT_DOUBLE_EQ(ub_merged[i + master->get_ncols()], ub_satellite[i]);
    }

    //Verify RHS
    DblVector rhs_merged(merged->get_nrows());
    DblVector rhs_master(master->get_nrows());
    DblVector rhs_satellite(satellite->get_nrows());
    merged->get_rhs(rhs_merged.data(), 0, merged->get_nrows() - 1);
    master->get_rhs(rhs_master.data(), 0, master->get_nrows() - 1);
    satellite->get_rhs(rhs_satellite.data(), 0, satellite->get_nrows() - 1);
    for (int i = 0; i < master->get_nrows(); ++i) {
        EXPECT_DOUBLE_EQ(rhs_merged[i], rhs_master[i]);
    }
    for (int i = 0; i < satellite->get_nrows(); ++i) {
        EXPECT_DOUBLE_EQ(rhs_merged[i + master->get_nrows()], rhs_satellite[i]);
    }

    //Verify row (LHS)
    auto [mstart_merged, mclind_merged, dmatval_merged, merge_ret] = get_rows(merged.get());
    auto [mstart_master, mclind_master, dmatval_master, master_ret] = get_rows(master.get());
    auto [mstart_satellite, mclind_satellite, dmatval_satellite, satellite_ret] = get_rows(satellite.get());
    for (int i = 0; i < master->get_nelems(); ++i) {
        EXPECT_DOUBLE_EQ(dmatval_merged[i], dmatval_master[i]);
    }
    for (int i = 0; i < satellite->get_nelems(); ++i) {
        EXPECT_DOUBLE_EQ(dmatval_merged[i + master->get_nelems()], dmatval_satellite[i]);
    }

    //RHS type
    std::vector<char> order_merged(merged->get_nrows());
    std::vector<char> order_master(master->get_nrows());
    std::vector<char> order_satellite(satellite->get_nrows());
    merged->get_row_type(order_merged.data(), 0, merged->get_nrows() - 1);
    master->get_row_type(order_master.data(), 0, master->get_nrows() - 1);
    satellite->get_row_type(order_satellite.data(), 0, satellite->get_nrows() - 1);
    for (int i = 0; i < master->get_nrows(); ++i) {
        EXPECT_EQ(order_merged[i], order_master[i]);
    }
    for (int i = 0; i < satellite->get_nrows(); ++i) {
        EXPECT_EQ(order_merged[i + master->get_nrows()], order_satellite[i]);
    }
}
