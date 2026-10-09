#include "EmptyLogManager.h"
#include "LoggerStub.h"
#include "NOOPSolver.h"
#include "antares-xpansion/benders/benders_core/BendersCutsManager.hxx"
#include "antares-xpansion/benders/benders_core/BendersMasterManager.h"
#include "antares-xpansion/benders/benders_core/IBendersProblemProvider.h"
#include "antares-xpansion/benders/benders_sequential/BendersCutsManagerSequential.h"
#include "antares-xpansion/benders/logger/Master.h"
#include "gtest/gtest.h"

namespace
{

// ─── NOOP problem provider ───
class NOOPProblemProviderForCuts: public IBendersProblemProvider
{
public:
    void provide_problem(const SolverIO&, std::shared_ptr<SolverAbstract>) const override
    {
    }

    std::filesystem::path provide_file_path() const override
    {
        return "";
    }
};

// ─── Helper to build a SubProblemData ───
PlainData::SubProblemData MakeSubProblemData(double cost,
                                             int simplex_iter,
                                             const Point& subgradient = {},
                                             double contribution_in_gap = 0.0)
{
    PlainData::SubProblemData spd;
    spd.subproblem_cost = cost;
    spd.simplex_iter = simplex_iter;
    spd.var_name_and_subgradient = subgradient;
    spd.contribution_in_gap = contribution_in_gap;
    spd.single_subpb_costs_under_approx = 0.0;
    spd.subproblem_timer = 0.0;
    spd.lpstatus = 0;
    return spd;
}

// ─── Helper to create a BendersMasterManager holding a master with NOOPSolver ───
std::unique_ptr<BendersMasterManager> MakeNOOPMasterManager(
  const VariableMap& var_map = {},
  const std::map<int, double>& subproblem_tolerance = {})
{
    static NOOPProblemProviderForCuts problem_provider;
    static EmptyLogManager solver_log_manager;
    auto master_manager = std::make_unique<BendersMasterManager>();
    master_manager->CreateMaster(var_map,
                                 "COIN",
                                 0,
                                 1,
                                 solver_log_manager,
                                 false,
                                 std::make_shared<xpansion::logger::Master>(),
                                 ProblemsFormat::MPS_FILE,
                                 &problem_provider,
                                 1e-4,
                                 subproblem_tolerance);
    master_manager->Master()->_solver = std::make_shared<NOOPSolver>();
    return master_manager;
}

// ─── Solver double recording the rows added to the master ───
class RowCapturingSolverForCuts: public NOOPSolver
{
public:
    struct CapturedRow
    {
        std::vector<char> rowtype;
        std::vector<double> rhs;
        std::vector<int> mclind;
        std::vector<double> matval;
    };

    std::vector<CapturedRow> captured_rows;

    void add_rows(int newrows,
                  int newnz,
                  const char* qrtype,
                  const double* rhs,
                  const double* /*range*/,
                  const int* /*mstart*/,
                  const int* mclind,
                  const double* dmatval,
                  const std::vector<std::string>& /*row_names*/) override
    {
        captured_rows.push_back({std::vector<char>(qrtype, qrtype + newrows),
                                 std::vector<double>(rhs, rhs + newrows),
                                 std::vector<int>(mclind, mclind + newnz),
                                 std::vector<double>(dmatval, dmatval + newnz)});
    }
};

// A master manager whose solver records the cuts, with x1 as the only candidate and one
// alpha_i column per subproblem.
std::unique_ptr<BendersMasterManager> MakeRecordingMasterManager(
  const std::map<int, double>& subproblem_tolerance,
  const std::vector<int>& alpha_ids,
  std::shared_ptr<RowCapturingSolverForCuts>& capturing_solver)
{
    auto master_manager = MakeNOOPMasterManager({{"x1", 0}}, subproblem_tolerance);
    capturing_solver = std::make_shared<RowCapturingSolverForCuts>();
    master_manager->Master()->_solver = capturing_solver;
    master_manager->Master()->_name_to_id = {{"x1", 0}};
    master_manager->Master()->set_id_single_subpb_costs_under_approx(alpha_ids);
    return master_manager;
}

// Minimal BendersCutsManager test double — exposes base class methods for testing.
class BendersCutsManagerTestDouble: public BendersCutsManager<BendersCutsManagerTestDouble>
{
public:
    using BendersCutsManager::BendersCutsManager;
};

} // namespace

// ═══════════════════════════════════════════════════════════
//  Base class: ComputeXCut (existing tests)
// ═══════════════════════════════════════════════════════════

TEST(ComputeXCutTest, FirstIteration)
{
    double sep_param = 0.8;
    double master_solution_tolerance = 0.1;

    CurrentIterationData data;
    data.solution.x_out = {{"x1", 1.0}, {"x2", 2.0}};
    data.solution.x_in = {{"x1", 3.0}, {"x2", 6.0}};
    data.solution.min_invest = {{"x1", -1e+20}, {"x2", -1e+20}};
    data.solution.max_invest = {{"x1", 1e+20}, {"x2", 1e+20}};
    data.control.it = 1;

    auto master_manager = MakeNOOPMasterManager();
    BendersCutsManagerTestDouble cuts_manager(*master_manager);
    cuts_manager.ComputeXCut(data, sep_param, master_solution_tolerance);

    // In first iteration, x_cut should equal x_out
    EXPECT_EQ(data.solution.x_cut, data.solution.x_out);
}

TEST(ComputeXCutTest, LaterIteration)
{
    double sep_param = 0.5;
    double master_solution_tolerance = 0.1;

    CurrentIterationData data;
    data.solution.x_out = {{"x1", 1.0}, {"x2", 2.0}};
    data.solution.x_in = {{"x1", 3.0}, {"x2", 6.0}};
    data.solution.min_invest = {{"x1", -1e+20}, {"x2", -1e+20}};
    data.solution.max_invest = {{"x1", 1e+20}, {"x2", 1e+20}};
    data.control.it = 2;

    auto master_manager = MakeNOOPMasterManager();
    BendersCutsManagerTestDouble cuts_manager(*master_manager);
    cuts_manager.ComputeXCut(data, sep_param, master_solution_tolerance);

    // x_cut = sep_param * x_out + (1 - sep_param) * x_in, no rounding here
    Point expected_x_cut = {{"x1", 2.0}, {"x2", 4.0}};
    EXPECT_EQ(data.solution.x_cut, expected_x_cut);
}

TEST(ComputeXCutTest, RoundingLowerBound)
{
    double sep_param = 0.5;
    double master_solution_tolerance = 0.1;

    CurrentIterationData data;
    data.solution.x_out = {{"x1", 1.0}, {"x2", 2.0}};
    data.solution.x_in = {{"x1", 1.01}, {"x2", 6.0}};
    data.solution.min_invest = {{"x1", 1}, {"x2", -1e+20}};
    data.solution.max_invest = {{"x1", 10}, {"x2", 1e+20}};
    data.control.it = 2;

    auto master_manager = MakeNOOPMasterManager();
    BendersCutsManagerTestDouble cuts_manager(*master_manager);
    cuts_manager.ComputeXCut(data, sep_param, master_solution_tolerance);

    // x1 rounded to lower bound (1.005 without rounding)
    Point expected_x_cut{{"x1", 1.0}, {"x2", 4.0}};
    EXPECT_EQ(data.solution.x_cut, expected_x_cut);
}

TEST(ComputeXCutTest, RoundingUpperBound)
{
    double sep_param = 0.5;
    double master_solution_tolerance = 0.1;

    CurrentIterationData data;
    data.solution.x_out = {{"x1", 1.0}, {"x2", 5.99}};
    data.solution.x_in = {{"x1", 3.0}, {"x2", 6.0}};
    data.solution.min_invest = {{"x1", 1}, {"x2", 0}};
    data.solution.max_invest = {{"x1", 10}, {"x2", 6.0}};
    data.control.it = 2;

    auto master_manager = MakeNOOPMasterManager();
    BendersCutsManagerTestDouble cuts_manager(*master_manager);
    cuts_manager.ComputeXCut(data, sep_param, master_solution_tolerance);

    // x1 not rounded, x2 rounded to upper bound (5.995 without rounding)
    Point expected_x_cut{{"x1", 2.0}, {"x2", 6.0}};
    EXPECT_EQ(data.solution.x_cut, expected_x_cut);
}

// ═══════════════════════════════════════════════════════════
//  Base class: ComputeXCut — master-only variables
// ═══════════════════════════════════════════════════════════

TEST(ComputeXCutTest, FirstIteration_CopiesMasterOnlyVars)
{
    CurrentIterationData data;
    data.solution.x_out = {{"x1", 1.0}};
    data.solution.x_in = {{"x1", 0.0}};
    data.solution.min_invest = {{"x1", -1e+20}};
    data.solution.max_invest = {{"x1", 1e+20}};
    data.solution.master_only_vars_out = {5.0, 10.0};
    data.solution.master_only_vars_in = {0.0, 0.0};
    data.solution.master_only_vars_cut.resize(2);
    data.control.it = 1;

    auto master_manager = MakeNOOPMasterManager();
    BendersCutsManagerTestDouble cuts_manager(*master_manager);
    cuts_manager.ComputeXCut(data, 0.5, 0.1);

    EXPECT_EQ(data.solution.master_only_vars_cut[0], 5.0);
    EXPECT_EQ(data.solution.master_only_vars_cut[1], 10.0);
    EXPECT_EQ(data.solution.master_only_vars_in[0], 5.0);
    EXPECT_EQ(data.solution.master_only_vars_in[1], 10.0);
}

TEST(ComputeXCutTest, LaterIteration_InterpolatesMasterOnlyVars)
{
    CurrentIterationData data;
    data.solution.x_out = {{"x1", 1.0}};
    data.solution.x_in = {{"x1", 1.0}};
    data.solution.min_invest = {{"x1", -1e+20}};
    data.solution.max_invest = {{"x1", 1e+20}};
    data.solution.master_only_vars_out = {10.0};
    data.solution.master_only_vars_in = {0.0};
    data.solution.master_only_vars_cut.resize(1);
    data.control.it = 2;

    auto master_manager = MakeNOOPMasterManager();
    BendersCutsManagerTestDouble cuts_manager(*master_manager);
    cuts_manager.ComputeXCut(data, 0.5, 0.1);

    // 0.5 * 10.0 + 0.5 * 0.0 = 5.0
    EXPECT_DOUBLE_EQ(data.solution.master_only_vars_cut[0], 5.0);
}

// ═══════════════════════════════════════════════════════════
//  Base class: SetSubproblemDataCostAndSimplexIter
// ═══════════════════════════════════════════════════════════

TEST(SetSubproblemDataCostAndSimplexIterTest, EmptyGatheredData)
{
    auto master_manager = MakeNOOPMasterManager();
    BendersCutsManagerTestDouble cuts_manager(*master_manager);
    CurrentIterationData data;
    data.cuts.subproblem_cost = 0.0;

    std::vector<SubProblemDataMap> gathered;
    cuts_manager.SetSubproblemDataCostAndSimplexIter(gathered, data);

    EXPECT_EQ(data.cuts.subproblem_cost, 0.0);
}

TEST(SetSubproblemDataCostAndSimplexIterTest, SingleSubproblem_AccumulatesCost)
{
    auto master_manager = MakeNOOPMasterManager();
    BendersCutsManagerTestDouble cuts_manager(*master_manager);
    CurrentIterationData data;
    data.cuts.subproblem_cost = 0.0;
    data.cuts.min_simplexiter = 999999;
    data.cuts.max_simplexiter = 0;

    SubProblemDataMap map1;
    map1["sp1"] = MakeSubProblemData(100.0, 50);

    std::vector<SubProblemDataMap> gathered = {map1};
    cuts_manager.SetSubproblemDataCostAndSimplexIter(gathered, data);

    EXPECT_DOUBLE_EQ(data.cuts.subproblem_cost, 100.0);
    EXPECT_EQ(data.cuts.max_simplexiter, 50);
    EXPECT_EQ(data.cuts.min_simplexiter, 50);
}

TEST(SetSubproblemDataCostAndSimplexIterTest, MultipleSubproblems_AccumulatesAll)
{
    auto master_manager = MakeNOOPMasterManager();
    BendersCutsManagerTestDouble cuts_manager(*master_manager);
    CurrentIterationData data;
    data.cuts.subproblem_cost = 0.0;
    data.cuts.min_simplexiter = 999999;
    data.cuts.max_simplexiter = 0;

    SubProblemDataMap map1;
    map1["sp1"] = MakeSubProblemData(100.0, 50);
    map1["sp2"] = MakeSubProblemData(200.0, 30);

    SubProblemDataMap map2;
    map2["sp3"] = MakeSubProblemData(150.0, 80);

    std::vector<SubProblemDataMap> gathered = {map1, map2};
    cuts_manager.SetSubproblemDataCostAndSimplexIter(gathered, data);

    EXPECT_DOUBLE_EQ(data.cuts.subproblem_cost, 450.0); // 100 + 200 + 150
    EXPECT_EQ(data.cuts.max_simplexiter, 80);
    EXPECT_EQ(data.cuts.min_simplexiter, 30);
}

TEST(SetSubproblemDataCostAndSimplexIterTest, PreExistingCost_IsAccumulated)
{
    auto master_manager = MakeNOOPMasterManager();
    BendersCutsManagerTestDouble cuts_manager(*master_manager);
    CurrentIterationData data;
    data.cuts.subproblem_cost = 50.0; // pre-existing
    data.cuts.min_simplexiter = 10;
    data.cuts.max_simplexiter = 100;

    SubProblemDataMap map1;
    map1["sp1"] = MakeSubProblemData(25.0, 60);

    std::vector<SubProblemDataMap> gathered = {map1};
    cuts_manager.SetSubproblemDataCostAndSimplexIter(gathered, data);

    EXPECT_DOUBLE_EQ(data.cuts.subproblem_cost, 75.0); // 50 + 25
    EXPECT_EQ(data.cuts.max_simplexiter, 100);         // 100 > 60, unchanged
    EXPECT_EQ(data.cuts.min_simplexiter, 10);          // 10 < 60, unchanged
}

// ═══════════════════════════════════════════════════════════
//  Base class: BuildAllAggregatedCuts
// ═══════════════════════════════════════════════════════════

TEST(BuildAllAggregatedCutsTest, EmptySubproblemNames_NoOp)
{
    auto master_manager = MakeNOOPMasterManager();
    BendersCutsManagerTestDouble cuts_manager(*master_manager);

    std::vector<SubProblemNamesInCut> subproblem_names; // empty
    std::vector<SubProblemDataMap> gathered;
    VariableMap problem_to_id;
    double ub = 0.0;
    Point x_cut;
    SubProblemDataMap cut_trace;

    cuts_manager
      .BuildAllAggregatedCuts(subproblem_names, gathered, problem_to_id, ub, x_cut, cut_trace);

    EXPECT_DOUBLE_EQ(ub, 0.0);
    EXPECT_TRUE(cut_trace.empty());
}

TEST(BuildAllAggregatedCutsTest, SingleCutGroup_AccumulatesUbAndTrace)
{
    VariableMap var_map = {{"x1", 0}};
    auto master_manager = MakeNOOPMasterManager(var_map, {{0, 1e-3}});
    BendersCutsManagerTestDouble cuts_manager(*master_manager);

    // One cut group with one subproblem at position 0 in gathered
    SubProblemNamesInCut cut_group = {{"sp1", 0}};
    std::vector<SubProblemNamesInCut> subproblem_names = {cut_group};

    SubProblemDataMap map1;
    map1["sp1"] = MakeSubProblemData(75.0, 10, {{"x1", 2.0}});

    std::vector<SubProblemDataMap> gathered = {map1};
    VariableMap problem_to_id = {{"sp1", 0}};
    double ub = 0.0;
    Point x_cut = {{"x1", 5.0}};
    SubProblemDataMap cut_trace;

    cuts_manager
      .BuildAllAggregatedCuts(subproblem_names, gathered, problem_to_id, ub, x_cut, cut_trace);

    EXPECT_DOUBLE_EQ(ub, 75.0);
    ASSERT_EQ(cut_trace.size(), 1u);
    EXPECT_DOUBLE_EQ(cut_trace.at("sp1").subproblem_cost, 75.0);
}

TEST(BuildAllAggregatedCutsTest, MultipleCutGroups_AccumulatesUb)
{
    VariableMap var_map = {{"x1", 0}};
    auto master_manager = MakeNOOPMasterManager(var_map, {{0, 1e-3}, {1, 1e-3}});
    BendersCutsManagerTestDouble cuts_manager(*master_manager);

    // Two cut groups, each with one subproblem
    SubProblemNamesInCut group1 = {{"sp1", 0}};
    SubProblemNamesInCut group2 = {{"sp2", 0}};
    std::vector<SubProblemNamesInCut> subproblem_names = {group1, group2};

    SubProblemDataMap map1;
    map1["sp1"] = MakeSubProblemData(30.0, 10, {{"x1", 1.0}});
    map1["sp2"] = MakeSubProblemData(50.0, 20, {{"x1", 3.0}});

    std::vector<SubProblemDataMap> gathered = {map1};
    VariableMap problem_to_id = {{"sp1", 0}, {"sp2", 1}};
    double ub = 0.0;
    Point x_cut = {{"x1", 1.0}};
    SubProblemDataMap cut_trace;

    cuts_manager
      .BuildAllAggregatedCuts(subproblem_names, gathered, problem_to_id, ub, x_cut, cut_trace);

    EXPECT_DOUBLE_EQ(ub, 80.0); // 30 + 50
    EXPECT_EQ(cut_trace.size(), 2u);
}

TEST(BuildAllAggregatedCutsTest, MultipleSubproblemsInOneCutGroup)
{
    VariableMap var_map = {{"x1", 0}};
    auto master_manager = MakeNOOPMasterManager(var_map, {{0, 1e-3}, {1, 1e-3}});
    BendersCutsManagerTestDouble cuts_manager(*master_manager);

    // One cut group with two subproblems from different ranks (positions)
    SubProblemNamesInCut group = {{"sp1", 0}, {"sp2", 1}};
    std::vector<SubProblemNamesInCut> subproblem_names = {group};

    SubProblemDataMap map_rank0;
    map_rank0["sp1"] = MakeSubProblemData(40.0, 10, {{"x1", 2.0}});

    SubProblemDataMap map_rank1;
    map_rank1["sp2"] = MakeSubProblemData(60.0, 20, {{"x1", 3.0}});

    std::vector<SubProblemDataMap> gathered = {map_rank0, map_rank1};
    VariableMap problem_to_id = {{"sp1", 0}, {"sp2", 1}};
    double ub = 0.0;
    Point x_cut = {{"x1", 1.0}};
    SubProblemDataMap cut_trace;

    cuts_manager
      .BuildAllAggregatedCuts(subproblem_names, gathered, problem_to_id, ub, x_cut, cut_trace);

    EXPECT_DOUBLE_EQ(ub, 100.0); // 40 + 60
    EXPECT_EQ(cut_trace.size(), 2u);
    EXPECT_DOUBLE_EQ(cut_trace.at("sp1").subproblem_cost, 40.0);
    EXPECT_DOUBLE_EQ(cut_trace.at("sp2").subproblem_cost, 60.0);
}

// One group per subproblem is the grouping obtained when NB_CUTS_PER_ITER == 0
// (SetAggregation then returns the subproblem count, giving groups of size 1). Each
// subproblem must get its own cut, on its own alpha_i and its own subgradient — the
// behaviour the removed ComputeCut path provided.
TEST(BuildAllAggregatedCutsTest, OneGroupPerSubproblem_EmitsOneCutPerSubproblem)
{
    std::shared_ptr<RowCapturingSolverForCuts> capturing_solver;
    auto master_manager = MakeRecordingMasterManager({{0, 1e-3}, {1, 1e-3}},
                                                     {10, 11},
                                                     capturing_solver);
    BendersCutsManagerTestDouble cuts_manager(*master_manager);

    std::vector<SubProblemNamesInCut> subproblem_names = {{{"sp1", 0}}, {{"sp2", 0}}};

    SubProblemDataMap map1;
    map1["sp1"] = MakeSubProblemData(30.0, 10, {{"x1", 1.0}});
    map1["sp2"] = MakeSubProblemData(50.0, 20, {{"x1", 3.0}});

    std::vector<SubProblemDataMap> gathered = {map1};
    VariableMap problem_to_id = {{"sp1", 0}, {"sp2", 1}};
    double ub = 0.0;
    Point x_cut = {{"x1", 1.0}};
    SubProblemDataMap cut_trace;

    cuts_manager
      .BuildAllAggregatedCuts(subproblem_names, gathered, problem_to_id, ub, x_cut, cut_trace);

    ASSERT_EQ(capturing_solver->captured_rows.size(), 2u);

    // -alpha_0 + 1.0 x1 <= -30 + 1.0 * 1.0
    const auto& cut_sp1 = capturing_solver->captured_rows[0];
    EXPECT_EQ(cut_sp1.rowtype, std::vector<char>({'L'}));
    EXPECT_DOUBLE_EQ(cut_sp1.rhs[0], -29.0);
    EXPECT_EQ(cut_sp1.mclind, std::vector<int>({0, 10}));
    EXPECT_EQ(cut_sp1.matval, std::vector<double>({1.0, -1.0}));

    // -alpha_1 + 3.0 x1 <= -50 + 3.0 * 1.0
    const auto& cut_sp2 = capturing_solver->captured_rows[1];
    EXPECT_DOUBLE_EQ(cut_sp2.rhs[0], -47.0);
    EXPECT_EQ(cut_sp2.mclind, std::vector<int>({0, 11}));
    EXPECT_EQ(cut_sp2.matval, std::vector<double>({3.0, -1.0}));
}

// A single group covering every subproblem is full aggregation. The cut carries the
// summed subgradient and rhs, and one -1 coefficient per alpha_i. Since the master
// enforces alpha = sum(alpha_i), this is the cut the removed ComputeCutAggregate path
// placed on alpha directly.
TEST(BuildAllAggregatedCutsTest, SingleGroupOverAllSubproblems_EmitsOneCutOnEveryAlpha)
{
    std::shared_ptr<RowCapturingSolverForCuts> capturing_solver;
    auto master_manager = MakeRecordingMasterManager({{0, 1e-3}, {1, 1e-3}},
                                                     {10, 11},
                                                     capturing_solver);
    BendersCutsManagerTestDouble cuts_manager(*master_manager);

    std::vector<SubProblemNamesInCut> subproblem_names = {{{"sp1", 0}, {"sp2", 0}}};

    SubProblemDataMap map1;
    map1["sp1"] = MakeSubProblemData(30.0, 10, {{"x1", 1.0}});
    map1["sp2"] = MakeSubProblemData(50.0, 20, {{"x1", 3.0}});

    std::vector<SubProblemDataMap> gathered = {map1};
    VariableMap problem_to_id = {{"sp1", 0}, {"sp2", 1}};
    double ub = 0.0;
    Point x_cut = {{"x1", 1.0}};
    SubProblemDataMap cut_trace;

    cuts_manager
      .BuildAllAggregatedCuts(subproblem_names, gathered, problem_to_id, ub, x_cut, cut_trace);

    ASSERT_EQ(capturing_solver->captured_rows.size(), 1u);

    // -alpha_0 - alpha_1 + 4.0 x1 <= -80 + 4.0 * 1.0
    // i.e. the sum of the two cuts emitted by the one-group-per-subproblem case above.
    const auto& cut = capturing_solver->captured_rows[0];
    EXPECT_EQ(cut.rowtype, std::vector<char>({'L'}));
    EXPECT_DOUBLE_EQ(cut.rhs[0], -76.0);
    EXPECT_EQ(cut.mclind, std::vector<int>({0, 10, 11}));
    EXPECT_EQ(cut.matval, std::vector<double>({4.0, -1.0, -1.0}));
}

// ═══════════════════════════════════════════════════════════
//  Sequential: SetSubproblemPerCutIndices
// ═══════════════════════════════════════════════════════════

TEST(BendersCutsManagerSequentialTest, SetSubproblemPerCutIndices_StoresConfig)
{
    CurrentIterationData data;
    VariableMap problem_to_id = {{"sp1", 0}};
    BendersRelevantIterationsData relevant_data;
    auto master_manager = MakeNOOPMasterManager({}, {{0, 1e-3}});

    BendersCutsManagerSequential seq_manager(data, problem_to_id, relevant_data, *master_manager);

    SubProblemNamesInCut group = {{"sp1", 0}};
    std::vector<SubProblemNamesInCut> indices = {group};
    seq_manager.SetSubproblemPerCutIndices(indices);

    // Verify it works by calling GatherAndBuildCutsImpl
    data.solution.x_cut = {{"x1", 1.0}};
    SubProblemDataMap sp_data;
    sp_data["sp1"] = MakeSubProblemData(100.0, 10, {{"x1", 2.0}});

    seq_manager.GatherAndBuildCutsImpl(sp_data);

    EXPECT_DOUBLE_EQ(data.cuts.ub, 100.0);
}

// ═══════════════════════════════════════════════════════════
//  Sequential: GatherAndBuildCutsImpl
// ═══════════════════════════════════════════════════════════

TEST(BendersCutsManagerSequentialTest, GatherAndBuildCuts_InitializesUbToZero)
{
    CurrentIterationData data;
    data.cuts.ub = 999.0; // should be reset
    VariableMap problem_to_id = {{"sp1", 0}};
    BendersRelevantIterationsData relevant_data;
    auto master_manager = MakeNOOPMasterManager();

    BendersCutsManagerSequential seq_manager(data, problem_to_id, relevant_data, *master_manager);
    seq_manager.SetSubproblemPerCutIndices({}); // empty => no cuts built

    SubProblemDataMap sp_data;
    seq_manager.GatherAndBuildCutsImpl(sp_data);

    EXPECT_DOUBLE_EQ(data.cuts.ub, 0.0); // reset to 0
}

TEST(BendersCutsManagerSequentialTest, GatherAndBuildCuts_AccumulatesUb)
{
    CurrentIterationData data;
    data.solution.x_cut = {{"x1", 1.0}};
    VariableMap problem_to_id = {{"sp1", 0}, {"sp2", 1}};
    BendersRelevantIterationsData relevant_data;
    auto master_manager = MakeNOOPMasterManager({{"x1", 0}}, {{0, 1e-3}, {1, 1e-3}});

    BendersCutsManagerSequential seq_manager(data, problem_to_id, relevant_data, *master_manager);

    SubProblemNamesInCut group1 = {{"sp1", 0}};
    SubProblemNamesInCut group2 = {{"sp2", 0}};
    seq_manager.SetSubproblemPerCutIndices({group1, group2});

    SubProblemDataMap sp_data;
    sp_data["sp1"] = MakeSubProblemData(30.0, 10, {{"x1", 1.0}});
    sp_data["sp2"] = MakeSubProblemData(70.0, 20, {{"x1", 2.0}});

    seq_manager.GatherAndBuildCutsImpl(sp_data);

    EXPECT_DOUBLE_EQ(data.cuts.ub, 100.0); // 30 + 70
}

TEST(BendersCutsManagerSequentialTest, GatherAndBuildCuts_PopulatesCutTrace)
{
    CurrentIterationData data;
    data.solution.x_cut = {{"x1", 1.0}};
    VariableMap problem_to_id = {{"sp1", 0}};
    BendersRelevantIterationsData relevant_data;
    auto master_manager = MakeNOOPMasterManager({{"x1", 0}}, {{0, 1e-3}});

    BendersCutsManagerSequential seq_manager(data, problem_to_id, relevant_data, *master_manager);

    SubProblemNamesInCut group = {{"sp1", 0}};
    seq_manager.SetSubproblemPerCutIndices({group});

    SubProblemDataMap sp_data;
    sp_data["sp1"] = MakeSubProblemData(42.0, 15, {{"x1", 3.0}});

    seq_manager.GatherAndBuildCutsImpl(sp_data);

    ASSERT_EQ(relevant_data.last._cut_trace.size(), 1u);
    EXPECT_DOUBLE_EQ(relevant_data.last._cut_trace.at("sp1").subproblem_cost, 42.0);
}

TEST(BendersCutsManagerSequentialTest, GatherAndBuildCuts_EmptyData_NoOp)
{
    CurrentIterationData data;
    VariableMap problem_to_id;
    BendersRelevantIterationsData relevant_data;
    auto master_manager = MakeNOOPMasterManager();

    BendersCutsManagerSequential seq_manager(data, problem_to_id, relevant_data, *master_manager);
    seq_manager.SetSubproblemPerCutIndices({});

    SubProblemDataMap sp_data; // empty
    seq_manager.GatherAndBuildCutsImpl(sp_data);

    EXPECT_DOUBLE_EQ(data.cuts.ub, 0.0);
    EXPECT_TRUE(relevant_data.last._cut_trace.empty());
}
