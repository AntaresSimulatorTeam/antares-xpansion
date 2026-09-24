#include <filesystem>
#include <vector>

#include "SolverMathOpt.h"
#include "gtest/gtest.h"

namespace fs = std::filesystem;

class SolverMathOptTest: public ::testing::Test
{
protected:
    void SetUp() override
    {
        solver_ = std::make_shared<SolverMathOpt>();
        solver_->init();
        mip_path_ = "data_test/mps/mip_toy_prob.mps";
        lp_path_ = "data_test/mps/lp_toy_prob.mps";
    }

    void TearDown() override
    {
        for (const auto& f : temp_files_)
        {
            std::error_code ec;
            fs::remove(f, ec);
        }
    }

    fs::path make_temp(const std::string& suffix)
    {
        auto p = fs::temp_directory_path()
                 / ("mathopt_test_" + std::to_string(counter_++) + suffix);
        temp_files_.push_back(p);
        return p;
    }

    std::shared_ptr<SolverMathOpt> solver_;
    fs::path mip_path_;
    fs::path lp_path_;

private:
    std::vector<fs::path> temp_files_;
    static int counter_;
};

int SolverMathOptTest::counter_ = 0;

// ---------------------------------------------------------------------------
// read_prob_mps
// ---------------------------------------------------------------------------

TEST_F(SolverMathOptTest, ReadProbMps_LoadsCorrectDimensions)
{
    solver_->read_prob_mps(mip_path_);
    EXPECT_EQ(solver_->get_ncols(), 2);
    EXPECT_EQ(solver_->get_nrows(), 2);
    EXPECT_EQ(solver_->get_nelems(), 4);
    EXPECT_EQ(solver_->get_n_integer_vars(), 2);
}

TEST_F(SolverMathOptTest, ReadProbMps_NonexistentFile_Throws)
{
    EXPECT_THROW(solver_->read_prob_mps("nonexistent.mps"), GenericSolverException);
}

TEST_F(SolverMathOptTest, ReadProbMps_AutoAppendsExtension)
{
    solver_->read_prob_mps("data_test/mps/mip_toy_prob");
    EXPECT_EQ(solver_->get_ncols(), 2);
    EXPECT_EQ(solver_->get_nrows(), 2);
}

// ---------------------------------------------------------------------------
// write_prob_mps / save_prob
// ---------------------------------------------------------------------------

TEST_F(SolverMathOptTest, WriteProbMps_RoundTrip)
{
    solver_->read_prob_mps(mip_path_);
    auto tmp = make_temp(".mps");
    solver_->write_prob_mps(tmp);

    auto solver2 = std::make_shared<SolverMathOpt>();
    solver2->init();
    solver2->read_prob_mps(tmp);

    EXPECT_EQ(solver2->get_ncols(), solver_->get_ncols());
    EXPECT_EQ(solver2->get_nrows(), solver_->get_nrows());
    EXPECT_EQ(solver2->get_nelems(), solver_->get_nelems());

    int ncols = solver_->get_ncols();
    std::vector<double> obj1(ncols), obj2(ncols);
    solver_->get_obj(obj1.data(), 0, ncols - 1);
    solver2->get_obj(obj2.data(), 0, ncols - 1);
    EXPECT_EQ(obj1, obj2);
}

TEST_F(SolverMathOptTest, SaveProb_DelegatesToWriteMps)
{
    solver_->read_prob_mps(mip_path_);
    auto tmp = make_temp(".mps");
    solver_->save_prob(tmp);

    auto solver2 = std::make_shared<SolverMathOpt>();
    solver2->init();
    solver2->read_prob_mps(tmp);

    EXPECT_EQ(solver2->get_ncols(), solver_->get_ncols());
    EXPECT_EQ(solver2->get_nrows(), solver_->get_nrows());
}

// ---------------------------------------------------------------------------
// write_prob_lp / read_prob_lp
// ---------------------------------------------------------------------------

TEST_F(SolverMathOptTest, WriteProbLp_RoundTrip)
{
    solver_->read_prob_mps(mip_path_);
    auto tmp = make_temp(".lp");
    solver_->write_prob_lp(tmp);

    auto solver2 = std::make_shared<SolverMathOpt>();
    solver2->init();
    solver2->read_prob_lp(tmp);

    EXPECT_EQ(solver2->get_ncols(), solver_->get_ncols());
    EXPECT_EQ(solver2->get_nrows(), solver_->get_nrows());
}

TEST_F(SolverMathOptTest, ReadProbLp_NonexistentFile_Throws)
{
    EXPECT_THROW(solver_->read_prob_lp("nonexistent.lp"), GenericSolverException);
}

// ---------------------------------------------------------------------------
// restore_prob
// ---------------------------------------------------------------------------

TEST_F(SolverMathOptTest, RestoreProb_DelegatesToReadMps)
{
    solver_->read_prob_mps(mip_path_);
    auto tmp = make_temp(".mps");
    solver_->write_prob_mps(tmp);

    auto solver2 = std::make_shared<SolverMathOpt>();
    solver2->init();
    solver2->restore_prob(tmp);

    EXPECT_EQ(solver2->get_ncols(), solver_->get_ncols());
    EXPECT_EQ(solver2->get_nrows(), solver_->get_nrows());
}

// ---------------------------------------------------------------------------
// write_basis / read_basis
// ---------------------------------------------------------------------------

TEST_F(SolverMathOptTest, WriteBasis_NoSolve_Throws)
{
    solver_->read_prob_mps(lp_path_);
    auto tmp = make_temp(".bas");
    EXPECT_THROW(solver_->write_basis(tmp), GenericSolverException);
}

TEST_F(SolverMathOptTest, WriteBasis_ReadBasis_RoundTrip)
{
    solver_->read_prob_mps(lp_path_);
    solver_->solve_lp();

    int ncols = solver_->get_ncols();
    int nrows = solver_->get_nrows();

    std::vector<int> cstatus1(ncols), rstatus1(nrows);
    solver_->get_basis(rstatus1.data(), cstatus1.data());

    auto tmp = make_temp(".bas");
    solver_->write_basis(tmp);

    auto solver2 = std::make_shared<SolverMathOpt>();
    solver2->init();
    solver2->read_prob_mps(lp_path_);
    solver2->read_basis(tmp);
    solver2->solve_lp();

    std::vector<int> cstatus2(ncols), rstatus2(nrows);
    solver2->get_basis(rstatus2.data(), cstatus2.data());

    EXPECT_EQ(cstatus1, cstatus2);
    EXPECT_EQ(rstatus1, rstatus2);
}

TEST_F(SolverMathOptTest, ReadBasis_NonexistentFile_Throws)
{
    solver_->read_prob_mps(lp_path_);
    EXPECT_THROW(solver_->read_basis("nonexistent_basis.bas"), GenericSolverException);
}

// ---------------------------------------------------------------------------
// set_basis
// ---------------------------------------------------------------------------

TEST_F(SolverMathOptTest, SetBasis_StoresBasisFromArrays)
{
    solver_->read_prob_mps(lp_path_);
    solver_->solve_lp();

    int ncols = solver_->get_ncols();
    int nrows = solver_->get_nrows();

    std::vector<int> cstatus(ncols), rstatus(nrows);
    solver_->get_basis(rstatus.data(), cstatus.data());

    auto solver2 = std::make_shared<SolverMathOpt>();
    solver2->init();
    solver2->read_prob_mps(lp_path_);
    solver2->set_basis(rstatus, cstatus);
    int status = solver2->solve_lp();
    EXPECT_EQ(status, 0); // OPTIMAL
}

// ===========================================================================
//
//  PROBLEM MODIFICATION TESTS
//
//  Uses mip_toy_prob.mps:
//    min -5*x1 - 4*x2
//    C0001:       x1 +  x2 <= 5
//    contrainte2: 10*x1 + 6*x2 <= 45
//    x1, x2 >= 0, integer
//
// ===========================================================================

TEST_F(SolverMathOptTest, DelRows_RemovesFirstRow)
{
    solver_->read_prob_mps(mip_path_);
    ASSERT_EQ(solver_->get_nrows(), 2);
    solver_->del_rows(0, 0);
    EXPECT_EQ(solver_->get_nrows(), 1);
    auto names = solver_->get_row_names(0, 0);
    EXPECT_EQ(names[0], "contrainte2");
}

TEST_F(SolverMathOptTest, DelCols_RemovesFirstCol)
{
    solver_->read_prob_mps(mip_path_);
    ASSERT_EQ(solver_->get_ncols(), 2);
    solver_->del_cols(0, 0);
    EXPECT_EQ(solver_->get_ncols(), 1);
    auto names = solver_->get_col_names(0, 0);
    EXPECT_EQ(names[0], "x2");
}

TEST_F(SolverMathOptTest, AddRows_AppendsConstraint)
{
    solver_->read_prob_mps(mip_path_);
    ASSERT_EQ(solver_->get_nrows(), 2);

    // Add one row: 2*x1 + 3*x2 <= 20
    char qrtype = 'L';
    double rhs = 20.0;
    int mstart[] = {0};
    int mclind[] = {0, 1};
    double dmatval[] = {2.0, 3.0};
    std::vector<std::string> names = {"new_row"};

    solver_->add_rows(1, 2, &qrtype, &rhs, nullptr, mstart, mclind, dmatval, names);

    EXPECT_EQ(solver_->get_nrows(), 3);
    EXPECT_EQ(solver_->get_row_index("new_row"), 2);

    std::vector<double> rhs_out(1);
    solver_->get_rhs(rhs_out.data(), 2, 2);
    EXPECT_DOUBLE_EQ(rhs_out[0], 20.0);
}

TEST_F(SolverMathOptTest, AddCols_AppendsVariable)
{
    solver_->read_prob_mps(mip_path_);
    ASSERT_EQ(solver_->get_ncols(), 2);

    // Add x3: obj=-1, lb=0, ub=10, coeff 1.0 in row 0, 2.0 in row 1
    double objx[] = {-1.0};
    int mstart[] = {0};
    int mrwind[] = {0, 1};
    double dmatval[] = {1.0, 2.0};
    double bdl[] = {0.0};
    double bdu[] = {10.0};
    std::vector<std::string> names = {"x3"};

    solver_->add_cols(1, 2, objx, mstart, mrwind, dmatval, bdl, bdu, names);

    EXPECT_EQ(solver_->get_ncols(), 3);
    EXPECT_EQ(solver_->get_col_index("x3"), 2);

    std::vector<double> lb(1);
    solver_->get_lb(lb.data(), 2, 2);
    EXPECT_DOUBLE_EQ(lb[0], 0.0);

    std::vector<double> ub(1);
    solver_->get_ub(ub.data(), 2, 2);
    EXPECT_DOUBLE_EQ(ub[0], 10.0);
}

TEST_F(SolverMathOptTest, AddName_Throws)
{
    solver_->read_prob_mps(mip_path_);
    EXPECT_THROW(solver_->add_name(0, "test", 0), NotImplementedFeatureSolverException);
}

TEST_F(SolverMathOptTest, AddNames_Throws)
{
    solver_->read_prob_mps(mip_path_);
    std::vector<std::string> names = {"a", "b"};
    EXPECT_THROW(solver_->add_names(0, names, 0, 1), NotImplementedFeatureSolverException);
}

TEST_F(SolverMathOptTest, ChgObj_SparseUpdate)
{
    solver_->read_prob_mps(mip_path_);
    solver_->chg_obj({1}, {7.0});

    std::vector<double> obj(2);
    solver_->get_obj(obj.data(), 0, 1);
    EXPECT_DOUBLE_EQ(obj[0], -5.0);
    EXPECT_DOUBLE_EQ(obj[1], 7.0);
}

TEST_F(SolverMathOptTest, ChgObjDirection_SwitchToMaximize)
{
    solver_->read_prob_mps(lp_path_);
    solver_->chg_obj_direction(false);

    int status = solver_->solve_lp();
    EXPECT_EQ(status, 0);
    double val = solver_->get_lp_value();
    EXPECT_DOUBLE_EQ(val, 0.0);
}

TEST_F(SolverMathOptTest, ChgBounds_SetLowerBound)
{
    solver_->read_prob_mps(mip_path_);
    solver_->chg_bounds({0}, {'L'}, {2.0});

    std::vector<double> lb(1);
    solver_->get_lb(lb.data(), 0, 0);
    EXPECT_DOUBLE_EQ(lb[0], 2.0);
}

TEST_F(SolverMathOptTest, ChgBounds_SetUpperBound)
{
    solver_->read_prob_mps(mip_path_);
    solver_->chg_bounds({1}, {'U'}, {3.0});

    std::vector<double> ub(1);
    solver_->get_ub(ub.data(), 1, 1);
    EXPECT_DOUBLE_EQ(ub[0], 3.0);
}

TEST_F(SolverMathOptTest, ChgBounds_SetBothBounds)
{
    solver_->read_prob_mps(mip_path_);
    solver_->chg_bounds({0}, {'B'}, {5.0});

    std::vector<double> lb(1), ub(1);
    solver_->get_lb(lb.data(), 0, 0);
    solver_->get_ub(ub.data(), 0, 0);
    EXPECT_DOUBLE_EQ(lb[0], 5.0);
    EXPECT_DOUBLE_EQ(ub[0], 5.0);
}

TEST_F(SolverMathOptTest, ChgColType_IntegerToContinuous)
{
    solver_->read_prob_mps(mip_path_);
    solver_->chg_col_type({0}, {'C'});

    std::vector<char> ct(1);
    solver_->get_col_type(ct.data(), 0, 0);
    EXPECT_EQ(ct[0], 'C');
    EXPECT_EQ(solver_->get_n_integer_vars(), 1);
}

TEST_F(SolverMathOptTest, ChgColType_ToBinary)
{
    solver_->read_prob_mps(mip_path_);
    solver_->chg_col_type({0}, {'B'});

    std::vector<char> ct(1);
    solver_->get_col_type(ct.data(), 0, 0);
    EXPECT_EQ(ct[0], 'B');

    std::vector<double> lb(1), ub(1);
    solver_->get_lb(lb.data(), 0, 0);
    solver_->get_ub(ub.data(), 0, 0);
    EXPECT_DOUBLE_EQ(lb[0], 0.0);
    EXPECT_DOUBLE_EQ(ub[0], 1.0);
}

TEST_F(SolverMathOptTest, ChgRhs_SingleRow)
{
    solver_->read_prob_mps(mip_path_);
    solver_->chg_rhs(0, 99.0);

    std::vector<double> rhs(1);
    solver_->get_rhs(rhs.data(), 0, 0);
    EXPECT_DOUBLE_EQ(rhs[0], 99.0);
}

TEST_F(SolverMathOptTest, ChgRhsValues_MultipleRows)
{
    solver_->read_prob_mps(mip_path_);
    std::vector<int> indices = {0, 1};
    std::vector<double> values = {10.0, 50.0};
    solver_->chg_rhs_values(indices, values);

    std::vector<double> rhs(2);
    solver_->get_rhs(rhs.data(), 0, 1);
    EXPECT_EQ(rhs, (std::vector<double>{10.0, 50.0}));
}

TEST_F(SolverMathOptTest, ChgCoef_SingleElement)
{
    solver_->read_prob_mps(mip_path_);
    solver_->chg_coef(0, 0, 7.0);

    std::vector<int> mstart(2);
    std::vector<int> mclind(2);
    std::vector<double> dmatval(2);
    int nels = 0;
    solver_->get_rows(mstart.data(), mclind.data(), dmatval.data(), 2, &nels, 0, 0);

    for (int k = 0; k < nels; ++k)
    {
        if (mclind[k] == 0)
        {
            EXPECT_DOUBLE_EQ(dmatval[k], 7.0);
        }
    }
}

TEST_F(SolverMathOptTest, ChgCoefs_MultipleElements)
{
    solver_->read_prob_mps(mip_path_);
    solver_->chg_coefs({0, 1}, {0, 1}, {7.0, 8.0});

    std::vector<int> mstart(2);
    std::vector<int> mclind(2);
    std::vector<double> dmatval(2);
    int nels = 0;

    solver_->get_rows(mstart.data(), mclind.data(), dmatval.data(), 2, &nels, 0, 0);
    for (int k = 0; k < nels; ++k)
    {
        if (mclind[k] == 0)
            EXPECT_DOUBLE_EQ(dmatval[k], 7.0);
    }

    solver_->get_rows(mstart.data(), mclind.data(), dmatval.data(), 2, &nels, 1, 1);
    for (int k = 0; k < nels; ++k)
    {
        if (mclind[k] == 1)
            EXPECT_DOUBLE_EQ(dmatval[k], 8.0);
    }
}

TEST_F(SolverMathOptTest, ChgRowName_UpdatesNameAndIndex)
{
    solver_->read_prob_mps(mip_path_);
    solver_->chg_row_name(0, "renamed_row");

    auto names = solver_->get_row_names(0, 0);
    EXPECT_EQ(names[0], "renamed_row");
    EXPECT_EQ(solver_->get_row_index("renamed_row"), 0);
    EXPECT_EQ(solver_->get_row_index("C0001"), -1);
}

TEST_F(SolverMathOptTest, ChgColName_UpdatesNameAndIndex)
{
    solver_->read_prob_mps(mip_path_);
    solver_->chg_col_name(0, "renamed_col");

    auto names = solver_->get_col_names(0, 0);
    EXPECT_EQ(names[0], "renamed_col");
    EXPECT_EQ(solver_->get_col_index("renamed_col"), 0);
    EXPECT_EQ(solver_->get_col_index("x1"), -1);
}

// ===========================================================================
//
//  SOLVING TESTS
//
//  solve_lp uses kGlop, solve_mip uses kHighs.
//  Both return OPTIMAL(0), INFEASIBLE(1), UNBOUNDED(2), or UNKNOWN.
//
//  Test data:
//    lp_toy_prob.mps  — feasible LP, optimal obj = -23.75
//    mip_toy_prob.mps — feasible MIP, optimal obj = -23
//    infeas.mps       — infeasible
//    unbounded.mps    — unbounded
//
// ===========================================================================

TEST_F(SolverMathOptTest, SolveLp_Optimal)
{
    solver_->read_prob_mps(lp_path_);
    int status = solver_->solve_lp();
    EXPECT_EQ(status, OPTIMAL);
}

TEST_F(SolverMathOptTest, SolveLp_Infeasible)
{
    solver_->read_prob_mps("data_test/mps/infeas.mps");
    int status = solver_->solve_lp();
    EXPECT_EQ(status, INFEASIBLE);
}

TEST_F(SolverMathOptTest, SolveLp_Unbounded)
{
    solver_->read_prob_mps("data_test/mps/unbounded.mps");
    int status = solver_->solve_lp();
    EXPECT_EQ(status, UNBOUNDED);
}

TEST_F(SolverMathOptTest, SolveMip_Optimal)
{
    solver_->read_prob_mps(mip_path_);
    int status = solver_->solve_mip();
    EXPECT_EQ(status, OPTIMAL);
}

TEST_F(SolverMathOptTest, SolveMip_Infeasible)
{
    solver_->read_prob_mps("data_test/mps/infeas.mps");
    int status = solver_->solve_mip();
    EXPECT_EQ(status, INFEASIBLE);
}

TEST_F(SolverMathOptTest, SolveMip_Unbounded)
{
    solver_->read_prob_mps("data_test/mps/unbounded.mps");
    int status = solver_->solve_mip();
    EXPECT_EQ(status, UNBOUNDED);
}
