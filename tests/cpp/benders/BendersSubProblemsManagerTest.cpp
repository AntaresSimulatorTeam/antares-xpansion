#include <fstream>

#include "antares-xpansion/benders/benders_sequential/BendersSubProblemsManagerSequential.hxx"
#include "antares-xpansion/benders/plugins/NoOperationPlugin.h"
#include "LoggerStub.h"
#include "WriterStub.h"
#include "NOOPSolver.h"
#include "gtest/gtest.h"

// ---------------------------------------------------------------------------
// Test helpers
// ---------------------------------------------------------------------------

/// A NOOPSolver that can return controlled values for SubproblemWorker usage.
/// SubproblemWorker::fix_to calls chg_bounds, solve calls solve_lp,
/// get_value calls get_lp_value, get_splex_num_of_ite_last delegates directly.
class ControllableSolver : public NOOPSolver
{
public:
    int get_ncols() const override { return ncols; }

    std::vector<std::string> get_col_names(int first, int last) const override
    {
        std::vector<std::string> result;
        for (int i = first; i <= last; ++i)
        {
            if (i < static_cast<int>(col_names.size()))
                result.push_back(col_names[i]);
            else
                result.push_back("col_" + std::to_string(i));
        }
        return result;
    }

    std::vector<std::string> get_col_names() override
    {
        return col_names;
    }

    void get_col_type(char* coltype, int first, int last) const override
    {
        for (int i = first; i <= last; ++i)
            coltype[i - first] = 'C'; // continuous
    }

    void get_lb(double* lb, int first, int last) const override
    {
        for (int i = first; i <= last; ++i)
            lb[i - first] = 0.0;
    }

    void get_ub(double* ub, int first, int last) const override
    {
        for (int i = first; i <= last; ++i)
            ub[i - first] = 1e20;
    }

    int solve_lp() override
    {
        ++solve_count;
        return 0; // optimal
    }

    double get_lp_value() const override { return lp_value; }

    int get_splex_num_of_ite_last() const override { return simplex_iters; }

    void get_lp_sol(double* primals, double* duals, double* reduced_costs) const override
    {
        // Return zeros — enough for get_subgradient to work
        for (int i = 0; i < ncols; ++i)
        {
            if (primals) primals[i] = 0.0;
            if (reduced_costs) reduced_costs[i] = 0.0;
        }
        for (int i = 0; i < nrows; ++i)
        {
            if (duals) duals[i] = dual_values.size() > static_cast<size_t>(i) ? dual_values[i] : 0.0;
        }
    }

    int get_nrows() const override { return nrows; }

    // Configurable state
    int ncols = 2;
    int nrows = 0;
    double lp_value = 42.0;
    int simplex_iters = 5;
    int solve_count = 0;
    std::vector<std::string> col_names = {"var1", "var2"};
    std::vector<double> dual_values;
};

// ---------------------------------------------------------------------------
// Helpers to build BendersBaseOptions (requires the inheritance chain)
// ---------------------------------------------------------------------------
static BendersBaseOptions MakeDefaultOptions()
{
    SolverBaseOptions solver_opts;
    solver_opts.LOG_LEVEL = 0;
    solver_opts.SOLVER_NAME = "COIN";
    solver_opts.SLAVE_WEIGHT = "CONSTANT";
    solver_opts.SLAVE_WEIGHT_VALUE = 1.0;

    BendersBaseOptions opts(solver_opts);
    opts.CACHE_PROBLEMS = 0; // fast path
    opts.MICRO_ITERATIONS = false;
    opts.BATCH_SIZE = 0;
    return opts;
}

// ---------------------------------------------------------------------------
// Test fixture
// ---------------------------------------------------------------------------
class BendersSubProblemsManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // 1. CurrentIterationData — default-constructed, set x_cut
        data_.x_cut = {{"var1", 1.0}, {"var2", 2.0}};
        data_.it = 1;

        // 2. BendersBaseOptions
        options_ = std::make_unique<BendersBaseOptions>(MakeDefaultOptions());

        // 3. Plugin (NoOperation — all callbacks are no-ops)
        plugin_ = std::make_shared<NoOperationPlugin>();

        // 4. Logger
        logger_ = std::make_shared<Xpansion::Test::LoggerNOOPStub>();

        // 5. SolverLogManager — default (no file)
        // solver_log_manager_ is default-constructed

        // 6. Writer
        writer_ = std::make_shared<Xpansion::Test::WriterNOOPStub>();

        // 7. should_parallelize
        should_parallelize_ = false;
    }

    /// Create the manager under test. Call after SetUp or after customizing options.
    BendersSubProblemsManagerSequential MakeManager()
    {
        return BendersSubProblemsManagerSequential(
            data_, *options_, plugin_, logger_,
            solver_log_manager_, writer_, should_parallelize_);
    }

    /// Create a SubproblemWorker backed by a ControllableSolver.
    std::shared_ptr<SubproblemWorker> MakeWorker(VariableMap variable_map)
    {
        auto solver = std::make_shared<ControllableSolver>();
        return std::make_shared<SubproblemWorker>(variable_map, solver, logger_);
    }

    /// Create a SubproblemWorker backed by the given solver.
    std::shared_ptr<SubproblemWorker> MakeWorker(
        VariableMap variable_map, std::shared_ptr<ControllableSolver> solver)
    {
        return std::make_shared<SubproblemWorker>(variable_map, solver, logger_);
    }

    // Test data
    CurrentIterationData data_;
    std::unique_ptr<BendersBaseOptions> options_;
    std::shared_ptr<BendersPlugin> plugin_;
    Logger logger_;
    SolverLogManager solver_log_manager_;
    std::shared_ptr<Output::OutputWriter> writer_;
    bool should_parallelize_ = false;
};

// ---------------------------------------------------------------------------
// Tests: construction
// ---------------------------------------------------------------------------
TEST_F(BendersSubProblemsManagerTest, CanBeConstructed)
{
    auto manager = MakeManager();
    // Should start with no subproblems
    EXPECT_TRUE(manager.GetSubProblemMap().empty());
    EXPECT_TRUE(manager.GetSubProblemNames().empty());
}

// ---------------------------------------------------------------------------
// Tests: Subproblem Registration
// ---------------------------------------------------------------------------
TEST_F(BendersSubProblemsManagerTest, AddSubproblemName_AddsToNamesList)
{
    auto manager = MakeManager();

    manager.AddSubproblemName("sub1.mps");
    manager.AddSubproblemName("sub2.mps");

    auto names = manager.GetSubProblemNames();
    ASSERT_EQ(names.size(), 2u);
    EXPECT_EQ(names[0], "sub1.mps");
    EXPECT_EQ(names[1], "sub2.mps");
}

TEST_F(BendersSubProblemsManagerTest, SetCouplingMap_StoresMap)
{
    auto manager = MakeManager();

    CouplingMap coupling_map;
    coupling_map["sub1.mps"] = {{"var1", 0}, {"var2", 1}};
    coupling_map["sub2.mps"] = {{"var1", 0}};
    manager.SetCouplingMap(coupling_map);

    const auto& stored = manager.GetCouplingMap();
    ASSERT_EQ(stored.size(), 2u);
    EXPECT_EQ(stored.at("sub1.mps").size(), 2u);
    EXPECT_EQ(stored.at("sub2.mps").size(), 1u);
}

// ---------------------------------------------------------------------------
// Tests: Problem-to-ID Mapping
// ---------------------------------------------------------------------------
TEST_F(BendersSubProblemsManagerTest, MatchProblemToId_AssignsSequentialIds)
{
    auto manager = MakeManager();

    CouplingMap coupling_map;
    coupling_map["alpha.mps"] = {{"v", 0}};
    coupling_map["beta.mps"] = {{"v", 0}};
    coupling_map["gamma.mps"] = {{"v", 0}};
    manager.SetCouplingMap(coupling_map);

    manager.MatchProblemToId();

    const auto& id_map = manager.GetProblemToId();
    ASSERT_EQ(id_map.size(), 3u);

    // std::map iterates in sorted order: alpha, beta, gamma → 0, 1, 2
    EXPECT_EQ(manager.ProblemToId("alpha.mps"), 0);
    EXPECT_EQ(manager.ProblemToId("beta.mps"), 1);
    EXPECT_EQ(manager.ProblemToId("gamma.mps"), 2);
}

TEST_F(BendersSubProblemsManagerTest, ProblemToId_ThrowsOnUnknownName)
{
    auto manager = MakeManager();
    manager.MatchProblemToId(); // empty coupling map → empty id map

    EXPECT_THROW(manager.ProblemToId("unknown"), std::out_of_range);
}

// ---------------------------------------------------------------------------
// Tests: Subproblem Weight
// ---------------------------------------------------------------------------
TEST_F(BendersSubProblemsManagerTest, SubproblemWeight_ConstantMode)
{
    options_->SLAVE_WEIGHT = "CONSTANT";
    options_->SLAVE_WEIGHT_VALUE = 2.0;
    auto manager = MakeManager();

    // CONSTANT mode: returns 1 / SLAVE_WEIGHT_VALUE
    EXPECT_DOUBLE_EQ(manager.SubproblemWeight(4, "any_name"), 0.5);
}

TEST_F(BendersSubProblemsManagerTest, SubproblemWeight_UniformMode)
{
    options_->SLAVE_WEIGHT = "UNIFORM";
    auto manager = MakeManager();

    // UNIFORM mode: returns 1 / subproblem_count
    EXPECT_DOUBLE_EQ(manager.SubproblemWeight(4, "any_name"), 0.25);
}

TEST_F(BendersSubProblemsManagerTest, SubproblemWeight_CustomWeights)
{
    options_->SLAVE_WEIGHT = "CUSTOM";
    options_->weights = {{"sub1.mps", 3.0}, {"sub2.mps", 7.0}};
    auto manager = MakeManager();

    // Custom mode: returns the weight from the map directly
    EXPECT_DOUBLE_EQ(manager.SubproblemWeight(2, "sub1.mps"), 3.0);
    EXPECT_DOUBLE_EQ(manager.SubproblemWeight(2, "sub2.mps"), 7.0);
}

// ---------------------------------------------------------------------------
// Tests: Dispatch Logic (GetSubproblemCut)
// ---------------------------------------------------------------------------
TEST_F(BendersSubProblemsManagerTest, GetSubproblemCut_DispatchesToFast_WhenCache0)
{
    options_->CACHE_PROBLEMS = 0;
    auto manager = MakeManager();

    bool fast_called = false;
    bool cache_called = false;

    FastBeginHook fast_hook = [&fast_called]()
    {
        fast_called = true;
        return std::vector<std::pair<std::string, SubproblemWorkerPtr>>{};
    };
    CacheBeginHook cache_hook = [&cache_called]()
    {
        cache_called = true;
        return std::vector<std::pair<std::string, VariableMap>>{};
    };
    PostSolveHook post_hook = [](const std::string&, PlainData::SubProblemData&,
                                 const SubproblemWorkerPtr&) {};

    SubProblemDataMap result;
    manager.GetSubproblemCut(result, fast_hook, cache_hook, post_hook);

    EXPECT_TRUE(fast_called);
    EXPECT_FALSE(cache_called);
}

TEST_F(BendersSubProblemsManagerTest, GetSubproblemCut_DispatchesToCache_WhenCache1)
{
    options_->CACHE_PROBLEMS = 1;
    auto manager = MakeManager();

    bool fast_called = false;
    bool cache_called = false;

    FastBeginHook fast_hook = [&fast_called]()
    {
        fast_called = true;
        return std::vector<std::pair<std::string, SubproblemWorkerPtr>>{};
    };
    CacheBeginHook cache_hook = [&cache_called]()
    {
        cache_called = true;
        return std::vector<std::pair<std::string, VariableMap>>{};
    };
    PostSolveHook post_hook = [](const std::string&, PlainData::SubProblemData&,
                                 const SubproblemWorkerPtr&) {};

    SubProblemDataMap result;
    manager.GetSubproblemCut(result, fast_hook, cache_hook, post_hook);

    EXPECT_FALSE(fast_called);
    EXPECT_TRUE(cache_called);
}

TEST_F(BendersSubProblemsManagerTest, GetSubproblemCut_DispatchesToSkeleton_WhenCache2)
{
    options_->CACHE_PROBLEMS = 2;
    auto manager = MakeManager();

    bool fast_called = false;
    bool cache_called = false;

    FastBeginHook fast_hook = [&fast_called]()
    {
        fast_called = true;
        return std::vector<std::pair<std::string, SubproblemWorkerPtr>>{};
    };
    // Skeleton path also uses the CacheBeginHook
    CacheBeginHook cache_hook = [&cache_called]()
    {
        cache_called = true;
        return std::vector<std::pair<std::string, VariableMap>>{};
    };
    PostSolveHook post_hook = [](const std::string&, PlainData::SubProblemData&,
                                 const SubproblemWorkerPtr&) {};

    SubProblemDataMap result;
    manager.GetSubproblemCut(result, fast_hook, cache_hook, post_hook);

    EXPECT_FALSE(fast_called);
    EXPECT_TRUE(cache_called);
}

TEST_F(BendersSubProblemsManagerTest, GetSubproblemCut_DoesNothing_WhenCacheUnknown)
{
    options_->CACHE_PROBLEMS = 99;
    auto manager = MakeManager();

    bool any_called = false;

    FastBeginHook fast_hook = [&any_called]()
    {
        any_called = true;
        return std::vector<std::pair<std::string, SubproblemWorkerPtr>>{};
    };
    CacheBeginHook cache_hook = [&any_called]()
    {
        any_called = true;
        return std::vector<std::pair<std::string, VariableMap>>{};
    };
    PostSolveHook post_hook = [](const std::string&, PlainData::SubProblemData&,
                                 const SubproblemWorkerPtr&) {};

    SubProblemDataMap result;
    manager.GetSubproblemCut(result, fast_hook, cache_hook, post_hook);

    EXPECT_FALSE(any_called);
}

// ---------------------------------------------------------------------------
// Tests: SolveSubproblem (core solving logic, shared by all paths)
// ---------------------------------------------------------------------------
TEST_F(BendersSubProblemsManagerTest, SolveSubproblem_CollectsCostSimplexItersAndSubgradient)
{
    auto manager = MakeManager();
    auto solver = std::make_shared<ControllableSolver>();
    solver->lp_value = 100.0;
    solver->simplex_iters = 12;
    VariableMap vars = {{"var1", 0}, {"var2", 1}};
    auto worker = MakeWorker(vars, solver);

    PlainData::SubProblemData result;
    manager.SolveSubproblem(result, "sub1.mps", worker, nullptr);

    EXPECT_DOUBLE_EQ(result.subproblem_cost, 100.0);
    EXPECT_EQ(result.simplex_iter, 12);
    EXPECT_GT(result.subproblem_timer, 0.0);
    EXPECT_EQ(result.lpstatus, 0);
    EXPECT_EQ(solver->solve_count, 1);
}

// ---------------------------------------------------------------------------
// Tests: GetSubproblemCutFast (fast path end-to-end via hooks)
// ---------------------------------------------------------------------------
TEST_F(BendersSubProblemsManagerTest, GetSubproblemCutFast_SolvesAllSubproblems)
{
    auto manager = MakeManager();

    auto solver1 = std::make_shared<ControllableSolver>();
    solver1->lp_value = 10.0;
    auto solver2 = std::make_shared<ControllableSolver>();
    solver2->lp_value = 20.0;

    VariableMap vars = {{"var1", 0}, {"var2", 1}};
    auto worker1 = MakeWorker(vars, solver1);
    auto worker2 = MakeWorker(vars, solver2);

    FastBeginHook hook = [&]()
    {
        return std::vector<std::pair<std::string, SubproblemWorkerPtr>>{
            {"sub1.mps", worker1}, {"sub2.mps", worker2}};
    };
    PostSolveHook post_hook = [](const std::string&, PlainData::SubProblemData&,
                                 const SubproblemWorkerPtr&) {};

    SubProblemDataMap result;
    manager.GetSubproblemCutFast(result, hook, post_hook);

    ASSERT_EQ(result.size(), 2u);
    EXPECT_DOUBLE_EQ(result["sub1.mps"].subproblem_cost, 10.0);
    EXPECT_DOUBLE_EQ(result["sub2.mps"].subproblem_cost, 20.0);
}

// ---------------------------------------------------------------------------
// Tests: GetSubproblemCutCache (CACHE_PROBLEMS=1, disk cache path)
// ---------------------------------------------------------------------------
TEST_F(BendersSubProblemsManagerTest, GetSubproblemCutCache_SolvesAllSubproblems)
{
    // Set up a temp directory with a minimal MPS file to satisfy makeSubproblemWorker
    auto tmp_dir = std::filesystem::temp_directory_path()
                   / ("benders_test_" + std::to_string(::getpid()));
    std::filesystem::create_directories(tmp_dir);

    // Write a minimal LP in MPS format with columns x1, x2
    {
        std::ofstream mps(tmp_dir / "sub1.mps");
        mps << "NAME SUB1\n"
            << "ROWS\n"
            << "    N OBJ\n"
            << "    L C1\n"
            << "COLUMNS\n"
            << "    x1 OBJ 1.0\n"
            << "    x1 C1  1.0\n"
            << "    x2 OBJ 2.0\n"
            << "    x2 C1  1.0\n"
            << "RHS\n"
            << "    RHS C1 10.0\n"
            << "BOUNDS\n"
            << "    LO BND x1 0.0\n"
            << "    LO BND x2 0.0\n"
            << "ENDATA\n";
    }
    {
        std::ofstream mps(tmp_dir / "sub2.mps");
        mps << "NAME SUB2\n"
            << "ROWS\n"
            << "    N OBJ\n"
            << "    L C1\n"
            << "COLUMNS\n"
            << "    x1 OBJ 3.0\n"
            << "    x1 C1  1.0\n"
            << "    x2 OBJ 4.0\n"
            << "    x2 C1  1.0\n"
            << "RHS\n"
            << "    RHS C1 10.0\n"
            << "BOUNDS\n"
            << "    LO BND x1 0.0\n"
            << "    LO BND x2 0.0\n"
            << "ENDATA\n";
    }

    options_->INPUTROOT = tmp_dir.string();
    options_->SOLVER_NAME = "COIN";
    options_->SLAVE_WEIGHT = "CONSTANT";
    options_->SLAVE_WEIGHT_VALUE = 1.0;
    data_.nsubproblem = 2;

    auto manager = MakeManager();

    // The CacheBeginHook provides (name, variable_map) pairs;
    // makeSubproblemWorker will load the MPS and create a real worker
    VariableMap vars = {{"x1", 0}, {"x2", 1}};
    CacheBeginHook hook = [&]()
    {
        return std::vector<std::pair<std::string, VariableMap>>{
            {"sub1.mps", vars}, {"sub2.mps", vars}};
    };
    PostSolveHook post_hook = [](const std::string&, PlainData::SubProblemData&,
                                 const SubproblemWorkerPtr&) {};

    SubProblemDataMap result;
    manager.GetSubproblemCutCache(result, hook, post_hook);

    ASSERT_EQ(result.size(), 2u);
    EXPECT_TRUE(result.count("sub1.mps"));
    EXPECT_TRUE(result.count("sub2.mps"));
    // Both should have been solved (lpstatus 0 = optimal)
    EXPECT_EQ(result["sub1.mps"].lpstatus, 0);
    EXPECT_EQ(result["sub2.mps"].lpstatus, 0);

    std::filesystem::remove_all(tmp_dir);
}

// ---------------------------------------------------------------------------
// Tests: GetCompactInMemCuts (CACHE_PROBLEMS=2, skeleton path)
// ---------------------------------------------------------------------------
TEST_F(BendersSubProblemsManagerTest, GetCompactInMemCuts_SolvesViaFactory)
{
    // Use the second SubproblemWorkerFactory constructor that takes a solver directly.
    // We need to set subproblem_worker_factory_ which is protected, so we use
    // BuildSubproblemWorkerFactory — but that reads files. Instead we create a
    // minimal fixture directory with skeleton files.

    auto tmp_dir = std::filesystem::temp_directory_path()
                   / ("benders_skel_test_" + std::to_string(::getpid()));
    auto sub_dir = tmp_dir / "sub";
    std::filesystem::create_directories(sub_dir);

    // Write skeleton MPS
    {
        std::ofstream mps(sub_dir / "sub.mps");
        mps << "NAME SKELETON\n"
            << "ROWS\n"
            << "    N OBJ\n"
            << "    L C1\n"
            << "COLUMNS\n"
            << "    x1 OBJ 1.0\n"
            << "    x1 C1  1.0\n"
            << "    x2 OBJ 2.0\n"
            << "    x2 C1  1.0\n"
            << "RHS\n"
            << "    RHS C1 10.0\n"
            << "BOUNDS\n"
            << "    LO BND x1 0.0\n"
            << "    LO BND x2 0.0\n"
            << "ENDATA\n";
    }

    // Write empty coefficient CSV files (no per-subproblem overrides)
    for (const auto& name: {"coef.csv", "obj_coef.csv", "rhs.csv"})
    {
        std::ofstream(sub_dir / name) << "\n";
    }
    for (const auto& name: {"coef_cols.csv", "coef_rows.csv", "obj_cols.csv", "rhs_rows.csv"})
    {
        std::ofstream(sub_dir / name) << "\n";
    }

    options_->INPUTROOT = tmp_dir.string();
    options_->SOLVER_NAME = "COIN";
    options_->SLAVE_WEIGHT = "CONSTANT";
    options_->SLAVE_WEIGHT_VALUE = 1.0;
    data_.nsubproblem = 1;

    auto manager = MakeManager();
    manager.AddSubproblemName("sub1.mps");
    manager.BuildSubproblemWorkerFactory(2);

    VariableMap vars = {{"x1", 0}, {"x2", 1}};
    CacheBeginHook hook = [&]()
    {
        return std::vector<std::pair<std::string, VariableMap>>{{"sub1.mps", vars}};
    };
    PostSolveHook post_hook = [](const std::string&, PlainData::SubProblemData&,
                                 const SubproblemWorkerPtr&) {};

    SubProblemDataMap result;
    manager.GetCompactInMemCuts(result, hook, post_hook);

    ASSERT_EQ(result.size(), 1u);
    EXPECT_TRUE(result.count("sub1.mps"));
    EXPECT_EQ(result["sub1.mps"].lpstatus, 0);

    std::filesystem::remove_all(tmp_dir);
}
