// ===========================================================================
//
//  Unit tests for SolverMathOpt getter functions.
//
//  All tests use data_test/mps/mip_toy_prob.mps (and sometimes
//  lp_toy_prob.mps for continuous-variable cases).
//
// ---------------------------------------------------------------------------
//  The optimization problem (mip_toy_prob.mps)
// ---------------------------------------------------------------------------
//
//    minimize   -5*x1 - 4*x2
//
//    subject to:
//      (C0001)        x1 +  x2  <= 5       row 0
//      (contrainte2)  10*x1 + 6*x2 <= 45   row 1
//
//      x1, x2 >= 0,  integer
//
//    Variables:   x1 (col 0),  x2 (col 1)
//    Constraints: C0001 (row 0),  contrainte2 (row 1)
//
//    Optimal solution: x1=3, x2=2, obj=-23
//
// ---------------------------------------------------------------------------
//  The constraint matrix A
// ---------------------------------------------------------------------------
//
//         x1    x2
//    C0001  [  1.0   1.0 ]
//    ctr2   [ 10.0   6.0 ]
//
//    Total nonzeros (nelems) = 4
//
// ---------------------------------------------------------------------------
//  Sparse row storage (Compressed Sparse Row — CSR)
// ---------------------------------------------------------------------------
//
//  The SolverAbstract interface stores the matrix row-by-row using three
//  parallel arrays plus a start-offset array:
//
//    mstart[i]   = index into mclind/dmatval where row i begins
//    mclind[k]   = column index of the k-th stored nonzero
//    dmatval[k]  = coefficient value of the k-th stored nonzero
//
//  mstart has (nrows + 1) entries. The last entry equals the total number
//  of stored nonzeros, so that the range for row i is
//      [mstart[i], mstart[i+1])
//
//  For our problem:
//
//    Row 0 (C0001):       1.0*x1 + 1.0*x2  ->  cols {0, 1}  vals {1.0, 1.0}
//    Row 1 (contrainte2): 10.0*x1 + 6.0*x2 ->  cols {0, 1}  vals {10.0, 6.0}
//
//    mstart  = { 0,  2,  4 }        row 0 at [0,2), row 1 at [2,4)
//    mclind  = { 0,  1,  0,  1 }    column indices
//    dmatval = { 1.0, 1.0, 10.0, 6.0 }   coefficient values
//
//  get_cols() returns the transposed view (Compressed Sparse Column), using
//  the same array layout but per-column instead of per-row.
//
// ===========================================================================

#include <filesystem>
#include <vector>

#include "SolverMathOpt.h"
#include "gtest/gtest.h"

namespace fs = std::filesystem;

class SolverMathOptGetterTest: public ::testing::Test
{
protected:
    void SetUp() override
    {
        solver_ = std::make_shared<SolverMathOpt>();
        solver_->init();
        mip_path_ = "data_test/mps/mip_toy_prob.mps";
        lp_path_ = "data_test/mps/lp_toy_prob.mps";
    }

    std::shared_ptr<SolverMathOpt> solver_;
    fs::path mip_path_;
    fs::path lp_path_;
};

// ===========================================================================
//  Dimensions
// ===========================================================================

TEST_F(SolverMathOptGetterTest, GetNcols_AfterReadMps)
{
    solver_->read_prob_mps(mip_path_);
    EXPECT_EQ(solver_->get_ncols(), 2);
}

TEST_F(SolverMathOptGetterTest, GetNrows_AfterReadMps)
{
    solver_->read_prob_mps(mip_path_);
    EXPECT_EQ(solver_->get_nrows(), 2);
}

TEST_F(SolverMathOptGetterTest, GetNelems_AfterReadMps)
{
    solver_->read_prob_mps(mip_path_);
    EXPECT_EQ(solver_->get_nelems(), 4);
}

TEST_F(SolverMathOptGetterTest, GetNIntegerVars_MipProblem)
{
    solver_->read_prob_mps(mip_path_);
    EXPECT_EQ(solver_->get_n_integer_vars(), 2);
}

TEST_F(SolverMathOptGetterTest, GetNIntegerVars_LpProblem)
{
    solver_->read_prob_mps(lp_path_);
    EXPECT_EQ(solver_->get_n_integer_vars(), 0);
}

// ===========================================================================
//  Objective
// ===========================================================================

TEST_F(SolverMathOptGetterTest, GetObj_FullRange)
{
    solver_->read_prob_mps(mip_path_);
    std::vector<double> obj(2);
    solver_->get_obj(obj.data(), 0, 1);
    EXPECT_EQ(obj, (std::vector<double>{-5.0, -4.0}));
}

TEST_F(SolverMathOptGetterTest, GetObj_PartialRange)
{
    solver_->read_prob_mps(mip_path_);
    std::vector<double> obj(1);
    solver_->get_obj(obj.data(), 1, 1);
    EXPECT_DOUBLE_EQ(obj[0], -4.0);
}

// ===========================================================================
//  Constraint matrix (sparse row / column views)
// ===========================================================================

TEST_F(SolverMathOptGetterTest, GetRows_FullRange)
{
    solver_->read_prob_mps(mip_path_);

    std::vector<int> mstart(3);     // nrows + 1
    std::vector<int> mclind(4);     // nelems
    std::vector<double> dmatval(4); // nelems
    int nels = 0;

    solver_->get_rows(mstart.data(), mclind.data(), dmatval.data(), 4, &nels, 0, 1);

    EXPECT_EQ(nels, 4);
    EXPECT_EQ(mstart, (std::vector<int>{0, 2, 4}));
    EXPECT_EQ(mclind, (std::vector<int>{0, 1, 0, 1}));
    EXPECT_EQ(dmatval, (std::vector<double>{1.0, 1.0, 10.0, 6.0}));
}

TEST_F(SolverMathOptGetterTest, GetRows_SingleRow)
{
    solver_->read_prob_mps(mip_path_);

    // Ask for row 1 only (contrainte2: 10*x1 + 6*x2)
    std::vector<int> mstart(2);     // 1 row + 1
    std::vector<int> mclind(2);
    std::vector<double> dmatval(2);
    int nels = 0;

    solver_->get_rows(mstart.data(), mclind.data(), dmatval.data(), 2, &nels, 1, 1);

    EXPECT_EQ(nels, 2);
    EXPECT_EQ(mstart, (std::vector<int>{0, 2}));
    EXPECT_EQ(mclind, (std::vector<int>{0, 1}));
    EXPECT_EQ(dmatval, (std::vector<double>{10.0, 6.0}));
}

TEST_F(SolverMathOptGetterTest, GetCols_FullRange)
{
    solver_->read_prob_mps(mip_path_);

    std::vector<int> mstart(3);     // ncols + 1
    std::vector<int> mrwind(4);     // nelems
    std::vector<double> dmatval(4); // nelems
    int nels = 0;

    solver_->get_cols(mstart.data(), mrwind.data(), dmatval.data(), 4, &nels, 0, 1);

    EXPECT_EQ(nels, 4);
    // Each column appears in both rows -> 2 entries per column
    EXPECT_EQ(mstart[0], 0);
    EXPECT_EQ(mstart[2], 4);
    EXPECT_EQ(mstart[1] - mstart[0], 2); // col 0: 2 entries
    EXPECT_EQ(mstart[2] - mstart[1], 2); // col 1: 2 entries
}

TEST_F(SolverMathOptGetterTest, GetCols_SingleCol)
{
    solver_->read_prob_mps(mip_path_);

    // Column 0 (x1): coeff 1.0 in row 0, coeff 10.0 in row 1
    std::vector<int> mstart(2);
    std::vector<int> mrwind(2);
    std::vector<double> dmatval(2);
    int nels = 0;

    solver_->get_cols(mstart.data(), mrwind.data(), dmatval.data(), 2, &nels, 0, 0);

    EXPECT_EQ(nels, 2);
    EXPECT_EQ(mstart, (std::vector<int>{0, 2}));
}

// ===========================================================================
//  Constraint info (row type, RHS, range)
// ===========================================================================

TEST_F(SolverMathOptGetterTest, GetRowType_AllRows)
{
    solver_->read_prob_mps(mip_path_);
    std::vector<char> rt(2);
    solver_->get_row_type(rt.data(), 0, 1);
    // Both constraints are <= (type 'L')
    EXPECT_EQ(rt, (std::vector<char>{'L', 'L'}));
}

TEST_F(SolverMathOptGetterTest, GetRhs_AllRows)
{
    solver_->read_prob_mps(mip_path_);
    std::vector<double> rhs(2);
    solver_->get_rhs(rhs.data(), 0, 1);
    EXPECT_EQ(rhs, (std::vector<double>{5.0, 45.0}));
}

TEST_F(SolverMathOptGetterTest, GetRhs_SingleRow)
{
    solver_->read_prob_mps(mip_path_);
    std::vector<double> rhs(1);
    solver_->get_rhs(rhs.data(), 1, 1);
    EXPECT_DOUBLE_EQ(rhs[0], 45.0);
}

TEST_F(SolverMathOptGetterTest, GetRhsRange_NonRangeRows)
{
    solver_->read_prob_mps(mip_path_);
    std::vector<double> range(2);
    solver_->get_rhs_range(range.data(), 0, 1);
    // No 'R'-type rows -> range is 0 for all
    EXPECT_EQ(range, (std::vector<double>{0.0, 0.0}));
}

// ===========================================================================
//  Variable info (column type, bounds)
// ===========================================================================

TEST_F(SolverMathOptGetterTest, GetColType_IntegerVars)
{
    solver_->read_prob_mps(mip_path_);
    std::vector<char> ct(2);
    solver_->get_col_type(ct.data(), 0, 1);
    EXPECT_EQ(ct, (std::vector<char>{'I', 'I'}));
}

TEST_F(SolverMathOptGetterTest, GetColType_ContinuousVars)
{
    solver_->read_prob_mps(lp_path_);
    std::vector<char> ct(2);
    solver_->get_col_type(ct.data(), 0, 1);
    EXPECT_EQ(ct, (std::vector<char>{'C', 'C'}));
}

TEST_F(SolverMathOptGetterTest, GetLb_AllVars)
{
    solver_->read_prob_mps(mip_path_);
    std::vector<double> lb(2);
    solver_->get_lb(lb.data(), 0, 1);
    EXPECT_EQ(lb, (std::vector<double>{0.0, 0.0}));
}

TEST_F(SolverMathOptGetterTest, GetUb_AllVars)
{
    solver_->read_prob_mps(mip_path_);
    std::vector<double> ub(2);
    solver_->get_ub(ub.data(), 0, 1);
    // MathOpt uses +infinity (not 1e20 like Xpress/CLP)
    EXPECT_GE(ub[0], 1e20);
    EXPECT_GE(ub[1], 1e20);
}

// ===========================================================================
//  Names and index lookups
// ===========================================================================

TEST_F(SolverMathOptGetterTest, GetColNames_FullRange)
{
    solver_->read_prob_mps(mip_path_);
    auto names = solver_->get_col_names(0, 1);
    ASSERT_EQ(names.size(), 2u);
    EXPECT_EQ(names[0], "x1");
    EXPECT_EQ(names[1], "x2");
}

TEST_F(SolverMathOptGetterTest, GetColNames_NoArgOverload)
{
    solver_->read_prob_mps(mip_path_);
    auto names = solver_->get_col_names();
    ASSERT_EQ(names.size(), 2u);
    EXPECT_EQ(names[0], "x1");
    EXPECT_EQ(names[1], "x2");
}

TEST_F(SolverMathOptGetterTest, GetRowNames_FullRange)
{
    solver_->read_prob_mps(mip_path_);
    auto names = solver_->get_row_names(0, 1);
    ASSERT_EQ(names.size(), 2u);
    EXPECT_EQ(names[0], "C0001");
    EXPECT_EQ(names[1], "contrainte2");
}

TEST_F(SolverMathOptGetterTest, GetRowNames_NoArgOverload)
{
    solver_->read_prob_mps(mip_path_);
    auto names = solver_->get_row_names();
    ASSERT_EQ(names.size(), 2u);
    EXPECT_EQ(names[0], "C0001");
    EXPECT_EQ(names[1], "contrainte2");
}

TEST_F(SolverMathOptGetterTest, GetColIndex_KnownName)
{
    solver_->read_prob_mps(mip_path_);
    EXPECT_EQ(solver_->get_col_index("x1"), 0);
    EXPECT_EQ(solver_->get_col_index("x2"), 1);
    EXPECT_EQ(solver_->get_col_index("bogus"), -1);
}

TEST_F(SolverMathOptGetterTest, GetRowIndex_KnownName)
{
    solver_->read_prob_mps(mip_path_);
    EXPECT_EQ(solver_->get_row_index("C0001"), 0);
    EXPECT_EQ(solver_->get_row_index("contrainte2"), 1);
    EXPECT_EQ(solver_->get_row_index("bogus"), -1);
}

// ===========================================================================
//  set_obj / set_obj_to_zero (in "Problem info" section alongside getters)
// ===========================================================================

TEST_F(SolverMathOptGetterTest, SetObj_ChangesObjective)
{
    solver_->read_prob_mps(mip_path_);
    double new_obj[] = {1.0, 2.0};
    solver_->set_obj(new_obj, 0, 1);

    std::vector<double> obj(2);
    solver_->get_obj(obj.data(), 0, 1);
    EXPECT_EQ(obj, (std::vector<double>{1.0, 2.0}));
}

TEST_F(SolverMathOptGetterTest, SetObj_PartialRange)
{
    solver_->read_prob_mps(mip_path_);
    double new_obj[] = {99.0};
    solver_->set_obj(new_obj, 1, 1);

    std::vector<double> obj(2);
    solver_->get_obj(obj.data(), 0, 1);
    EXPECT_DOUBLE_EQ(obj[0], -5.0); // unchanged
    EXPECT_DOUBLE_EQ(obj[1], 99.0);
}

TEST_F(SolverMathOptGetterTest, SetObjToZero_ClearsAllCoefficients)
{
    solver_->read_prob_mps(mip_path_);
    solver_->set_obj_to_zero();

    std::vector<double> obj(2);
    solver_->get_obj(obj.data(), 0, 1);
    EXPECT_EQ(obj, (std::vector<double>{0.0, 0.0}));
}

// ===========================================================================
//  Solution info (require solving first)
// ===========================================================================

TEST_F(SolverMathOptGetterTest, GetLpValue_AfterSolve)
{
    solver_->read_prob_mps(lp_path_);
    solver_->solve_lp();
    EXPECT_DOUBLE_EQ(solver_->get_lp_value(), -23.75);
}

TEST_F(SolverMathOptGetterTest, GetLpValue_NoSolve_Throws)
{
    solver_->read_prob_mps(lp_path_);
    EXPECT_THROW(solver_->get_lp_value(), std::exception);
}

TEST_F(SolverMathOptGetterTest, GetMipValue_AfterSolve)
{
    solver_->read_prob_mps(mip_path_);
    solver_->solve_mip();
    EXPECT_DOUBLE_EQ(solver_->get_mip_value(), -23.0);
}

TEST_F(SolverMathOptGetterTest, GetMipValue_NoSolve_Throws)
{
    solver_->read_prob_mps(mip_path_);
    EXPECT_THROW(solver_->get_mip_value(), std::exception);
}

TEST_F(SolverMathOptGetterTest, GetLpSol_Primals)
{
    solver_->read_prob_mps(lp_path_);
    solver_->solve_lp();

    std::vector<double> primals(2);
    solver_->get_lp_sol(primals.data(), nullptr, nullptr);
    EXPECT_DOUBLE_EQ(primals[0], 3.75);
    EXPECT_DOUBLE_EQ(primals[1], 1.25);
}

TEST_F(SolverMathOptGetterTest, GetLpSol_Duals)
{
    solver_->read_prob_mps(lp_path_);
    solver_->solve_lp();

    std::vector<double> duals(2);
    solver_->get_lp_sol(nullptr, duals.data(), nullptr);
    // Dual values exist after LP solve (exact values depend on solver)
    // Just verify they are finite
    EXPECT_TRUE(std::isfinite(duals[0]));
    EXPECT_TRUE(std::isfinite(duals[1]));
}

TEST_F(SolverMathOptGetterTest, GetLpSol_ReducedCosts)
{
    solver_->read_prob_mps(lp_path_);
    solver_->solve_lp();

    std::vector<double> rc(2);
    solver_->get_lp_sol(nullptr, nullptr, rc.data());
    EXPECT_TRUE(std::isfinite(rc[0]));
    EXPECT_TRUE(std::isfinite(rc[1]));
}

TEST_F(SolverMathOptGetterTest, GetLpSol_NoSolve_Throws)
{
    solver_->read_prob_mps(lp_path_);
    std::vector<double> primals(2);
    EXPECT_THROW(solver_->get_lp_sol(primals.data(), nullptr, nullptr), std::exception);
}

TEST_F(SolverMathOptGetterTest, GetMipSol_Primals)
{
    solver_->read_prob_mps(mip_path_);
    solver_->solve_mip();

    std::vector<double> primals(2);
    solver_->get_mip_sol(primals.data());
    EXPECT_DOUBLE_EQ(primals[0], 3.0);
    EXPECT_DOUBLE_EQ(primals[1], 2.0);
}

TEST_F(SolverMathOptGetterTest, GetMipSol_NoSolve_Throws)
{
    solver_->read_prob_mps(mip_path_);
    std::vector<double> primals(2);
    EXPECT_THROW(solver_->get_mip_sol(primals.data()), std::exception);
}

TEST_F(SolverMathOptGetterTest, GetBasis_AfterLpSolve)
{
    solver_->read_prob_mps(lp_path_);
    solver_->solve_lp();

    std::vector<int> rstatus(2);
    std::vector<int> cstatus(2);
    solver_->get_basis(rstatus.data(), cstatus.data());
    // After optimal LP solve, basis statuses should be valid (0-4 range)
    for (int s : rstatus)
    {
        EXPECT_GE(s, 0);
        EXPECT_LE(s, 4);
    }
    for (int s : cstatus)
    {
        EXPECT_GE(s, 0);
        EXPECT_LE(s, 4);
    }
}

TEST_F(SolverMathOptGetterTest, GetBasis_NoSolve_ReturnsZeros)
{
    solver_->read_prob_mps(lp_path_);

    std::vector<int> rstatus(2, -1);
    std::vector<int> cstatus(2, -1);
    solver_->get_basis(rstatus.data(), cstatus.data());
    EXPECT_EQ(rstatus, (std::vector<int>{0, 0}));
    EXPECT_EQ(cstatus, (std::vector<int>{0, 0}));
}

TEST_F(SolverMathOptGetterTest, GetSplexNumOfIteLastSolve_AfterLpSolve)
{
    solver_->read_prob_mps(lp_path_);
    solver_->solve_lp();
    int iters = solver_->get_splex_num_of_ite_last();
    EXPECT_GE(iters, 0);
}

TEST_F(SolverMathOptGetterTest, GetSplexNumOfIteLastSolve_NoSolve_ReturnsZero)
{
    solver_->read_prob_mps(lp_path_);
    EXPECT_EQ(solver_->get_splex_num_of_ite_last(), 0);
}

TEST_F(SolverMathOptGetterTest, GetPresolveMap_Throws)
{
    solver_->read_prob_mps(mip_path_);
    std::vector<int> rowmap(2);
    std::vector<int> colmap(2);
    EXPECT_THROW(solver_->get_presolve_map(rowmap.data(), colmap.data()), std::exception);
}

// ===========================================================================
//  Parameters (setters)
// ===========================================================================

TEST_F(SolverMathOptGetterTest, SetOutputLogLevel_DoesNotThrow)
{
    solver_->read_prob_mps(lp_path_);
    EXPECT_NO_THROW(solver_->set_output_log_level(0));
    EXPECT_NO_THROW(solver_->set_output_log_level(3));
}

TEST_F(SolverMathOptGetterTest, SetAlgorithm_DualAccepted)
{
    EXPECT_NO_THROW(solver_->set_algorithm("DUAL"));
    EXPECT_NO_THROW(solver_->set_algorithm("dual"));
}

TEST_F(SolverMathOptGetterTest, SetAlgorithm_UnsupportedThrows)
{
    EXPECT_THROW(solver_->set_algorithm("BARRIER"), std::exception);
}

TEST_F(SolverMathOptGetterTest, SetThreads_DoesNotThrow)
{
    EXPECT_NO_THROW(solver_->set_threads(4));
}

TEST_F(SolverMathOptGetterTest, SetOptimalityGap_DoesNotThrow)
{
    EXPECT_NO_THROW(solver_->set_optimality_gap(1e-6));
}

TEST_F(SolverMathOptGetterTest, SetSimplexIter_LimitsSolve)
{
    solver_->read_prob_mps(lp_path_);
    solver_->set_simplex_iter(1);
    solver_->solve_lp();
    int iters = solver_->get_splex_num_of_ite_last();
    EXPECT_LE(iters, 1);
}

TEST_F(SolverMathOptGetterTest, MarkIndicesToKeepPresolve_Throws)
{
    solver_->read_prob_mps(mip_path_);
    std::vector<int> rowind(2), colind(2);
    EXPECT_THROW(solver_->mark_indices_to_keep_presolve(2, 2, rowind.data(), colind.data()),
                 std::exception);
}

TEST_F(SolverMathOptGetterTest, PresolveOnly_Throws)
{
    EXPECT_THROW(solver_->presolve_only(), std::exception);
}
