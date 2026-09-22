#include "antares-xpansion/benders/benders_core/BendersMasterManager.h"

#include <cmath>
#include <memory>

#include "EmptyLogManager.h"
#include "LoggerStub.h"
#include "NOOPSolver.h"
#include "WriterStub.h"
#include "antares-xpansion/benders/benders_core/IBendersProblemProvider.h"
#include "antares-xpansion/benders/benders_core/WorkerMaster.h"
#include "antares-xpansion/benders/logger/Master.h"
#include "gtest/gtest.h"

namespace
{

// ─── NOOP problem provider for constructing WorkerMaster without real files ───
class NOOPProblemProvider : public IBendersProblemProvider
{
public:
    void provide_problem(const SolverIO&, std::shared_ptr<SolverAbstract>) const override {}
    std::filesystem::path provide_file_path() const override { return ""; }
};

// ─── Configurable mock solver ───
class MockSolverForMasterManager : public NOOPSolver
{
public:
    int ncols_ = 3;
    int nrows_ = 2;
    int nelems_ = 4;
    std::vector<double> obj_coeffs_ = {1.0, 2.0, 3.0};
    std::vector<double> rhs_vals_;
    double lp_value_ = 42.0;
    std::vector<double> lp_solution_;
    std::vector<double> lb_vals_;
    std::vector<double> ub_vals_;

    // Tracking mutation calls
    int set_obj_calls = 0;
    std::vector<double> last_set_obj;
    int add_rows_calls = 0;
    int chg_rhs_calls = 0;
    int write_basis_calls = 0;

    int get_ncols() const override { return ncols_; }
    int get_nrows() const override { return nrows_; }
    int get_nelems() const override { return nelems_; }

    void get_obj(double* obj, int first, int last) const override
    {
        for (int i = first; i <= last && i < static_cast<int>(obj_coeffs_.size()); ++i)
        {
            obj[i - first] = obj_coeffs_[i];
        }
    }

    void set_obj(const double* obj, int first, int last) override
    {
        ++set_obj_calls;
        last_set_obj.assign(obj, obj + (last - first + 1));
    }

    double get_lp_value() const override { return lp_value_; }

    void get_lp_sol(double* primals, double*, double*) const override
    {
        if (!lp_solution_.empty())
        {
            std::copy(lp_solution_.begin(), lp_solution_.end(), primals);
        }
    }

    void get_rhs(double* rhs, int first, int /*last*/) const override
    {
        if (!rhs_vals_.empty())
        {
            *rhs = rhs_vals_[first];
        }
    }

    void get_lb(double* lb, int first, int last) const override
    {
        if (!lb_vals_.empty())
        {
            for (int i = first; i <= last; ++i)
            {
                lb[i - first] = lb_vals_[i];
            }
        }
    }

    void get_ub(double* ub, int first, int last) const override
    {
        if (!ub_vals_.empty())
        {
            for (int i = first; i <= last; ++i)
            {
                ub[i - first] = ub_vals_[i];
            }
        }
    }

    void add_rows(int, int, const char*, const double*, const double*, const int*, const int*,
                  const double*, const std::vector<std::string>&) override
    {
        ++add_rows_calls;
    }

    void chg_rhs(int, double) override { ++chg_rhs_calls; }

    void write_basis(const std::filesystem::path&) override { ++write_basis_calls; }

    void get_row_type(char* qrtype, int first, int last) const override
    {
        for (int i = first; i <= last; ++i)
        {
            qrtype[i - first] = 'L';
        }
    }

    void get_rows(int* mstart, int* mclind, double* dmatval, int, int* nels, int first,
                  int last) const override
    {
        // Minimal stub: 1 element per row
        for (int r = first; r <= last; ++r)
        {
            int idx = r - first;
            mstart[idx] = idx;
            mclind[idx] = 0;
            dmatval[idx] = 1.0;
            nels[idx] = 1;
        }
    }

    SolverAbstract* clone() const override { return nullptr; }
};

// ─── Fixture ───
class BendersMasterManagerTest : public ::testing::Test
{
public:
    BendersMasterManager manager_;
    std::shared_ptr<MockSolverForMasterManager> mock_solver_;
    NOOPProblemProvider problem_provider_;
    EmptyLogManager solver_log_manager_;

    void SetUp() override { mock_solver_ = std::make_shared<MockSolverForMasterManager>(); }

    // Creates a WorkerMaster via CreateMaster, then injects the mock solver
    void CreateAndInjectMockSolver(const VariableMap& var_map = {{"var0", 0}, {"var1", 1}, {"var2", 2}})
    {
        Logger logger = std::make_shared<xpansion::logger::Master>();
        std::map<int, double> subproblem_tolerance;
        manager_.CreateMaster(var_map, "COIN", 0, 1, solver_log_manager_, false, logger,
                              ProblemsFormat::MPS_FILE, &problem_provider_, 1e-4,
                              subproblem_tolerance);
        // Inject mock solver
        manager_.GetMaster()->_solver = mock_solver_;
        // Set up id_to_name for WorkerMaster
        for (const auto& [name, id] : var_map)
        {
            manager_.GetMaster()->_id_to_name[id] = name;
        }
        manager_.GetMaster()->_name_to_id = var_map;
    }
};

// ═══════════════════════════════════════════════════════════
//  Lifecycle: IsEmpty, CreateMaster, FreeMaster
// ═══════════════════════════════════════════════════════════

TEST_F(BendersMasterManagerTest, DefaultState_IsEmpty)
{
    ASSERT_TRUE(manager_.IsEmpty());
    ASSERT_EQ(manager_.GetMaster(), nullptr);
}

TEST_F(BendersMasterManagerTest, CreateMaster_SetsNotEmpty)
{
    CreateAndInjectMockSolver();
    ASSERT_FALSE(manager_.IsEmpty());
    ASSERT_NE(manager_.GetMaster(), nullptr);
}

TEST_F(BendersMasterManagerTest, FreeMaster_SetsEmpty)
{
    CreateAndInjectMockSolver();
    manager_.FreeMaster();
    ASSERT_TRUE(manager_.IsEmpty());
}

// ═══════════════════════════════════════════════════════════
//  VariableMap
// ═══════════════════════════════════════════════════════════

TEST_F(BendersMasterManagerTest, SetAndGetVariableMap)
{
    VariableMap vm = {{"x", 0}, {"y", 1}};
    manager_.SetVariableMap(vm);

    ASSERT_EQ(manager_.GetVariableMap().size(), 2u);
    ASSERT_EQ(manager_.GetVariableMap().at("x"), 0);
    ASSERT_EQ(manager_.GetVariableMap().at("y"), 1);
}

TEST_F(BendersMasterManagerTest, CreateMaster_SetsVariableMap)
{
    VariableMap vm = {{"a", 0}, {"b", 1}};
    CreateAndInjectMockSolver(vm);

    ASSERT_EQ(manager_.GetVariableMap().at("a"), 0);
    ASSERT_EQ(manager_.GetVariableMap().at("b"), 1);
}

// ═══════════════════════════════════════════════════════════
//  GetMasterPath
// ═══════════════════════════════════════════════════════════

TEST_F(BendersMasterManagerTest, GetMasterPath_MPS_Format)
{
    auto path = manager_.GetMasterPath("/input", "master", ProblemsFormat::MPS_FILE, "COIN");
    ASSERT_EQ(path, std::filesystem::path("/input/master.mps"));
}

TEST_F(BendersMasterManagerTest, GetMasterPath_Optimized_Xpress)
{
    auto path = manager_.GetMasterPath("/input", "master", ProblemsFormat::OPTIMIZED, "XPRESS");
    ASSERT_EQ(path, std::filesystem::path("/input/master.svf"));
}

TEST_F(BendersMasterManagerTest, GetMasterPath_Optimized_NonXpress_FallsBackToMPS)
{
    auto path = manager_.GetMasterPath("/input", "master", ProblemsFormat::OPTIMIZED, "COIN");
    ASSERT_EQ(path, std::filesystem::path("/input/master.mps"));
}

// ═══════════════════════════════════════════════════════════
//  Solver delegation: GetNrows, GetNcols, GetNElems
// ═══════════════════════════════════════════════════════════

TEST_F(BendersMasterManagerTest, GetNrows_DelegatesToSolver)
{
    CreateAndInjectMockSolver();
    mock_solver_->nrows_ = 10;
    ASSERT_EQ(manager_.GetNrows(), 10);
}

TEST_F(BendersMasterManagerTest, GetNcols_DelegatesToSolver)
{
    CreateAndInjectMockSolver();
    mock_solver_->ncols_ = 7;
    ASSERT_EQ(manager_.GetNcols(), 7);
}

TEST_F(BendersMasterManagerTest, GetNElems_DelegatesToSolver)
{
    CreateAndInjectMockSolver();
    mock_solver_->nelems_ = 42;
    ASSERT_EQ(manager_.GetNElems(), 42);
}

// ═══════════════════════════════════════════════════════════
//  Objective function
// ═══════════════════════════════════════════════════════════

TEST_F(BendersMasterManagerTest, GetObjectiveFunctionCoeffs_ReturnsSolverObj)
{
    CreateAndInjectMockSolver();
    mock_solver_->obj_coeffs_ = {10.0, 20.0, 30.0};
    mock_solver_->ncols_ = 3;

    auto obj = manager_.GetObjectiveFunctionCoeffs();

    ASSERT_EQ(obj.size(), 3u);
    ASSERT_EQ(obj[0], 10.0);
    ASSERT_EQ(obj[1], 20.0);
    ASSERT_EQ(obj[2], 30.0);
}

TEST_F(BendersMasterManagerTest, SetObjectiveFunction_DelegatesToSolver)
{
    CreateAndInjectMockSolver();
    std::vector<double> coeffs = {5.0, 6.0};

    manager_.SetObjectiveFunction(coeffs.data(), 0, 1);

    ASSERT_EQ(mock_solver_->set_obj_calls, 1);
    ASSERT_EQ(mock_solver_->last_set_obj.size(), 2u);
    ASSERT_EQ(mock_solver_->last_set_obj[0], 5.0);
    ASSERT_EQ(mock_solver_->last_set_obj[1], 6.0);
}

TEST_F(BendersMasterManagerTest, SetObjectiveFunctionCoeffsToZeros_SetsAllToZero)
{
    VariableMap vm = {{"a", 0}, {"b", 1}, {"c", 2}};
    CreateAndInjectMockSolver(vm);

    manager_.SetObjectiveFunctionCoeffsToZeros();

    ASSERT_EQ(mock_solver_->set_obj_calls, 1);
    ASSERT_EQ(mock_solver_->last_set_obj.size(), 3u);
    for (const auto& v : mock_solver_->last_set_obj)
    {
        ASSERT_EQ(v, 0.0);
    }
}

// ═══════════════════════════════════════════════════════════
//  ChangeRhs / GetRhs
// ═══════════════════════════════════════════════════════════

TEST_F(BendersMasterManagerTest, ChangeRhs_DelegatesToSolver)
{
    CreateAndInjectMockSolver();
    manager_.ChangeRhs(0, 99.0);
    ASSERT_EQ(mock_solver_->chg_rhs_calls, 1);
}

TEST_F(BendersMasterManagerTest, GetRhs_DelegatesToSolver)
{
    CreateAndInjectMockSolver();
    mock_solver_->rhs_vals_ = {42.0, 55.0};

    double rhs = 0.0;
    manager_.GetRhs(rhs, 0);
    ASSERT_EQ(rhs, 42.0);
}

// ═══════════════════════════════════════════════════════════
//  AddRows
// ═══════════════════════════════════════════════════════════

TEST_F(BendersMasterManagerTest, AddRows_DelegatesToSolver)
{
    CreateAndInjectMockSolver();

    std::vector<char> qrtype = {'L'};
    std::vector<double> rhs = {10.0};
    std::vector<double> range = {0.0};
    std::vector<int> mstart = {0};
    std::vector<int> mclind = {0};
    std::vector<double> dmatval = {1.0};

    manager_.AddRows(qrtype, rhs, range, mstart, mclind, dmatval);

    ASSERT_EQ(mock_solver_->add_rows_calls, 1);
}

// ═══════════════════════════════════════════════════════════
//  GetRowsCoeffs / GetRowType
// ═══════════════════════════════════════════════════════════

TEST_F(BendersMasterManagerTest, GetRowsCoeffs_DelegatesToSolver)
{
    CreateAndInjectMockSolver();

    std::vector<int> mstart(1);
    std::vector<int> mclind(1);
    std::vector<double> dmatval(1);
    std::vector<int> nels(1);

    manager_.GetRowsCoeffs(mstart, mclind, dmatval, 1, nels, 0, 0);

    ASSERT_EQ(nels[0], 1);
    ASSERT_EQ(dmatval[0], 1.0);
}

TEST_F(BendersMasterManagerTest, GetRowType_DelegatesToSolver)
{
    CreateAndInjectMockSolver();

    std::vector<char> qrtype(1);
    manager_.GetRowType(qrtype, 0, 0);

    ASSERT_EQ(qrtype[0], 'L');
}

// ═══════════════════════════════════════════════════════════
//  WriteBasis
// ═══════════════════════════════════════════════════════════

TEST_F(BendersMasterManagerTest, WriteBasis_DelegatesToSolver)
{
    CreateAndInjectMockSolver();
    manager_.WriteBasis("/tmp/basis.bss");
    ASSERT_EQ(mock_solver_->write_basis_calls, 1);
}

// ═══════════════════════════════════════════════════════════
//  GetNameToId / GetMasterOnlyVarsIds
// ═══════════════════════════════════════════════════════════

TEST_F(BendersMasterManagerTest, GetNameToId_ReturnsWorkerMasterMap)
{
    VariableMap vm = {{"x", 0}, {"y", 1}};
    CreateAndInjectMockSolver(vm);

    const auto& name_to_id = manager_.GetNameToId();
    ASSERT_EQ(name_to_id.at("x"), 0);
    ASSERT_EQ(name_to_id.at("y"), 1);
}

TEST_F(BendersMasterManagerTest, GetMasterOnlyVarsIds_ReturnsWorkerMasterIds)
{
    CreateAndInjectMockSolver();
    manager_.GetMaster()->_id_master_only_vars = {5, 10, 15};

    const auto& ids = manager_.GetMasterOnlyVarsIds();
    ASSERT_EQ(ids.size(), 3u);
    ASSERT_EQ(ids[0], 5);
    ASSERT_EQ(ids[1], 10);
    ASSERT_EQ(ids[2], 15);
}

// ═══════════════════════════════════════════════════════════
//  ComputeInvestCost
// ═══════════════════════════════════════════════════════════

TEST_F(BendersMasterManagerTest, ComputeInvestCost_ComputesDotProduct)
{
    VariableMap vm = {{"a", 0}, {"b", 1}};
    CreateAndInjectMockSolver(vm);
    mock_solver_->obj_coeffs_ = {3.0, 5.0};
    mock_solver_->ncols_ = 2;
    manager_.GetMaster()->_id_master_only_vars = {};

    CurrentIterationData data{};
    data.solution.x_cut = {{"a", 2.0}, {"b", 4.0}};

    manager_.ComputeInvestCost(data);

    // 3.0*2.0 + 5.0*4.0 = 6 + 20 = 26
    ASSERT_DOUBLE_EQ(data.master.invest_cost, 26.0);
}

TEST_F(BendersMasterManagerTest, ComputeInvestCost_IncludesMasterOnlyVars)
{
    VariableMap vm = {{"a", 0}};
    CreateAndInjectMockSolver(vm);
    mock_solver_->obj_coeffs_ = {3.0, 7.0};
    mock_solver_->ncols_ = 2;
    manager_.GetMaster()->_id_master_only_vars = {1};

    CurrentIterationData data{};
    data.solution.x_cut = {{"a", 2.0}};
    data.solution.master_only_vars_cut = {5.0};

    manager_.ComputeInvestCost(data);

    // 3.0*2.0 + 7.0*5.0 = 6 + 35 = 41
    ASSERT_DOUBLE_EQ(data.master.invest_cost, 41.0);
}

// ═══════════════════════════════════════════════════════════
//  UpdateOverallCosts
// ═══════════════════════════════════════════════════════════

TEST_F(BendersMasterManagerTest, UpdateOverallCosts_ComputesFromVariableMap)
{
    VariableMap vm = {{"x", 0}, {"y", 1}};
    CreateAndInjectMockSolver(vm);
    mock_solver_->obj_coeffs_ = {2.0, 4.0};
    mock_solver_->ncols_ = 2;
    manager_.GetMaster()->_id_master_only_vars = {};

    CurrentIterationData data{};
    data.solution.x_cut = {{"x", 3.0}, {"y", 5.0}};
    double best_invest_cost = 0.0;

    manager_.UpdateOverallCosts(data, best_invest_cost);

    // 2.0*3.0 + 4.0*5.0 = 6 + 20 = 26
    ASSERT_DOUBLE_EQ(data.master.invest_cost, 26.0);
    ASSERT_DOUBLE_EQ(best_invest_cost, 26.0);
}

} // anonymous namespace
