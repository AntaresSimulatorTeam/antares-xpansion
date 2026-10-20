#include "antares-xpansion/benders/benders_core/BendersOuterLoopManager.h"

#include <memory>

#include "InMemoryWriter.h"
#include "WriterStub.h"
#include "antares-xpansion/benders/output/OutputWriter.h"
#include "gtest/gtest.h"

namespace
{

Output::SolutionData MakeDummySolution(int best_it = 1, int nb_weeks = 52)
{
    Output::SolutionData sol;
    sol.best_it = best_it;
    sol.nbWeeks_p = nb_weeks;
    sol.problem_status = "OPTIMAL";
    sol.stopping_criterion = "relative_gap";
    return sol;
}

Output::Iteration MakeDummyIteration()
{
    Output::Iteration iter{};
    iter.lb = 100.0;
    iter.ub = 200.0;
    iter.best_ub = 150.0;
    iter.master_duration = 1.0;
    iter.subproblem_duration = 2.0;
    iter.investment_cost = 50.0;
    iter.operational_cost = 100.0;
    iter.overall_cost = 150.0;
    iter.optimality_gap = 50.0;
    iter.relative_gap = 0.33;
    iter.cumulative_number_of_subproblem_resolved = 10;
    return iter;
}

// ─── Recording writer: captures calls for assertions ───
class RecordingWriter : public Xpansion::Test::WriterNOOPStub
{
public:
    int write_solution_calls = 0;
    int write_iteration_calls = 0;
    int dump_calls = 0;
    Output::SolutionData last_solution;
    size_t last_iteration_num = 0;

    void write_solution(const Output::SolutionData& solution) override
    {
        ++write_solution_calls;
        last_solution = solution;
    }

    void write_iteration(const Output::Iteration& iteration_data,
                         const size_t iteration_num) override
    {
        ++write_iteration_calls;
        last_iteration_num = iteration_num;
    }

    void dump() override { ++dump_calls; }
};

// ─── Fixture ───
class BendersOuterLoopManagerTest : public ::testing::Test
{
public:
    CurrentIterationData data_;
    BendersRelevantIterationsData relevant_data_;
    std::shared_ptr<RecordingWriter> writer_;

    int solution_fn_call_count_ = 0;
    int iteration_fn_call_count_ = 0;

    void SetUp() override
    {
        data_ = {};
        relevant_data_ = {};
        writer_ = std::make_shared<RecordingWriter>();
    }

    std::shared_ptr<BendersOuterLoopManager> MakeManager()
    {
        return MakeManager(
          [this]()
          {
              ++solution_fn_call_count_;
              return MakeDummySolution();
          },
          [this](const WorkerMasterData&)
          {
              ++iteration_fn_call_count_;
              return MakeDummyIteration();
          });
    }

    std::shared_ptr<BendersOuterLoopManager> MakeManager(
      BendersOuterLoopManager::BendersSolutionFn sol_fn,
      BendersOuterLoopManager::IterationFn iter_fn)
    {
        return std::make_shared<BendersOuterLoopManager>(data_, relevant_data_, writer_,
                                                         std::move(sol_fn), std::move(iter_fn));
    }
};

// ═══════════════════════════════════════════════════════════
//  GetOuterLoopData
// ═══════════════════════════════════════════════════════════

TEST_F(BendersOuterLoopManagerTest, GetOuterLoopData_ReturnsDefaultCriteria)
{
    auto mgr = MakeManager();
    auto result = mgr->GetOuterLoopData();
    ASSERT_EQ(result.benders_num_run, 0);
    ASSERT_TRUE(result.criteria.empty());
    ASSERT_EQ(result.max_criterion, 0.0);
    ASSERT_EQ(result.max_criterion_area, "N/A");
}

TEST_F(BendersOuterLoopManagerTest, GetOuterLoopData_ReflectsMutationsOnData)
{
    auto mgr = MakeManager();
    data_.criteria.benders_num_run = 5;
    data_.criteria.max_criterion = 42.0;
    data_.criteria.max_criterion_area = "area_x";

    auto result = mgr->GetOuterLoopData();
    ASSERT_EQ(result.benders_num_run, 5);
    ASSERT_EQ(result.max_criterion, 42.0);
    ASSERT_EQ(result.max_criterion_area, "area_x");
}

// ═══════════════════════════════════════════════════════════
//  SetBilevelBestub
// ═══════════════════════════════════════════════════════════

TEST_F(BendersOuterLoopManagerTest, SetBilevelBestub_UpdatesCriteriaField)
{
    auto mgr = MakeManager();
    mgr->SetBilevelBestub(999.0);
    ASSERT_EQ(data_.criteria.outer_loop_bilevel_best_ub, 999.0);
}

// ═══════════════════════════════════════════════════════════
//  PushCriteriaForIteration / ClearCriteriaHistory
// ═══════════════════════════════════════════════════════════

TEST_F(BendersOuterLoopManagerTest, PushAndClear_EmptyHistory)
{
    auto mgr = MakeManager();
    // Initially empty — GetOuterLoopCriterionAtBestBenders returns empty
    ASSERT_TRUE(mgr->GetOuterLoopCriterionAtBestBenders().empty());
}

TEST_F(BendersOuterLoopManagerTest, PushCriteria_AccumulatesHistory)
{
    auto mgr = MakeManager();
    mgr->PushCriteriaForIteration({1.0, 2.0});
    mgr->PushCriteriaForIteration({3.0, 4.0});
    mgr->PushCriteriaForIteration({5.0, 6.0});

    // best_it is 1-indexed; set to 2 to get second entry
    data_.control.best_it = 2;
    auto result = mgr->GetOuterLoopCriterionAtBestBenders();
    ASSERT_EQ(result.size(), 2u);
    ASSERT_EQ(result[0], 3.0);
    ASSERT_EQ(result[1], 4.0);
}

TEST_F(BendersOuterLoopManagerTest, ClearCriteriaHistory_EmptiesHistory)
{
    auto mgr = MakeManager();
    mgr->PushCriteriaForIteration({1.0});
    mgr->PushCriteriaForIteration({2.0});
    mgr->ClearCriteriaHistory();

    ASSERT_TRUE(mgr->GetOuterLoopCriterionAtBestBenders().empty());
}

// ═══════════════════════════════════════════════════════════
//  GetOuterLoopCriterionAtBestBenders
// ═══════════════════════════════════════════════════════════

TEST_F(BendersOuterLoopManagerTest, GetCriterionAtBestBenders_FirstIteration)
{
    auto mgr = MakeManager();
    mgr->PushCriteriaForIteration({10.0, 20.0});
    data_.control.best_it = 1;

    auto result = mgr->GetOuterLoopCriterionAtBestBenders();
    ASSERT_EQ(result.size(), 2u);
    ASSERT_EQ(result[0], 10.0);
    ASSERT_EQ(result[1], 20.0);
}

TEST_F(BendersOuterLoopManagerTest, GetCriterionAtBestBenders_LastIteration)
{
    auto mgr = MakeManager();
    mgr->PushCriteriaForIteration({1.0});
    mgr->PushCriteriaForIteration({2.0});
    mgr->PushCriteriaForIteration({3.0});
    data_.control.best_it = 3;

    auto result = mgr->GetOuterLoopCriterionAtBestBenders();
    ASSERT_EQ(result.size(), 1u);
    ASSERT_EQ(result[0], 3.0);
}

// ═══════════════════════════════════════════════════════════
//  UpdateOuterLoopSolution / GetOuterLoopSolution
// ═══════════════════════════════════════════════════════════

TEST_F(BendersOuterLoopManagerTest, UpdateAndGetSolution_CallsFnAndSetsBestIt)
{
    data_.criteria.benders_num_run = 7;
    auto mgr = MakeManager();

    mgr->UpdateOuterLoopSolution();

    ASSERT_EQ(solution_fn_call_count_, 1);
    auto sol = mgr->GetOuterLoopSolution();
    ASSERT_EQ(sol.best_it, 7); // overwritten with benders_num_run
    ASSERT_EQ(sol.problem_status, "OPTIMAL");
}

TEST_F(BendersOuterLoopManagerTest, UpdateSolution_CalledMultipleTimes)
{
    data_.criteria.benders_num_run = 1;
    auto mgr = MakeManager();
    mgr->UpdateOuterLoopSolution();

    data_.criteria.benders_num_run = 3;
    mgr->UpdateOuterLoopSolution();

    ASSERT_EQ(solution_fn_call_count_, 2);
    ASSERT_EQ(mgr->GetOuterLoopSolution().best_it, 3);
}

// ═══════════════════════════════════════════════════════════
//  SaveOuterLoopSolutionInOutputFile
// ═══════════════════════════════════════════════════════════

TEST_F(BendersOuterLoopManagerTest, SaveSolution_WritesToWriter)
{
    data_.criteria.benders_num_run = 2;
    auto mgr = MakeManager();
    mgr->UpdateOuterLoopSolution();
    mgr->SaveOuterLoopSolutionInOutputFile();

    ASSERT_EQ(writer_->write_solution_calls, 1);
    ASSERT_EQ(writer_->dump_calls, 1);
    ASSERT_EQ(writer_->last_solution.best_it, 2);
}

// ═══════════════════════════════════════════════════════════
//  SaveCurrentOuterLoopIterationInOutputFile
// ═══════════════════════════════════════════════════════════

TEST_F(BendersOuterLoopManagerTest, SaveIteration_SkipsWhenLastDataInvalid)
{
    relevant_data_.last._valid = false;
    auto mgr = MakeManager();

    mgr->SaveCurrentOuterLoopIterationInOutputFile();

    ASSERT_EQ(writer_->write_iteration_calls, 0);
    ASSERT_EQ(writer_->dump_calls, 0);
    ASSERT_EQ(iteration_fn_call_count_, 0);
}

TEST_F(BendersOuterLoopManagerTest, SaveIteration_WritesWhenLastDataValid)
{
    relevant_data_.last._valid = true;
    data_.criteria.benders_num_run = 4;
    auto mgr = MakeManager();

    mgr->SaveCurrentOuterLoopIterationInOutputFile();

    ASSERT_EQ(writer_->write_iteration_calls, 1);
    ASSERT_EQ(writer_->dump_calls, 1);
    ASSERT_EQ(writer_->last_iteration_num, 4u);
    ASSERT_EQ(iteration_fn_call_count_, 1);
}

// ═══════════════════════════════════════════════════════════
//  SetCriterionComputationInputs / GetCriterionComputation
// ═══════════════════════════════════════════════════════════

TEST_F(BendersOuterLoopManagerTest, CriterionComputation_DefaultIsEmpty)
{
    auto mgr = MakeManager();
    ASSERT_TRUE(mgr->GetCriterionComputation().IsEmpty());
}

TEST_F(BendersOuterLoopManagerTest, SetCriterionInputs_MakesNonEmpty)
{
    auto mgr = MakeManager();

    Benders::Criterion::CriterionInputData input_data;
    input_data.AddSingleData(
      Benders::Criterion::CriterionSingleInputData("UnsuppliedEnergy::", "area1", 100.0));

    mgr->SetCriterionComputationInputs(input_data);

    ASSERT_FALSE(mgr->GetCriterionComputation().IsEmpty());
    ASSERT_EQ(mgr->GetCriterionComputation().getCriterionInputData().Criteria().size(), 1u);
}

TEST_F(BendersOuterLoopManagerTest, SetCriterionInputs_ReplacesExisting)
{
    auto mgr = MakeManager();

    Benders::Criterion::CriterionInputData input1;
    input1.AddSingleData(
      Benders::Criterion::CriterionSingleInputData("prefix::", "area1", 100.0));
    mgr->SetCriterionComputationInputs(input1);

    Benders::Criterion::CriterionInputData input2;
    input2.AddSingleData(
      Benders::Criterion::CriterionSingleInputData("prefix::", "area_a", 50.0));
    input2.AddSingleData(
      Benders::Criterion::CriterionSingleInputData("prefix::", "area_b", 75.0));
    mgr->SetCriterionComputationInputs(input2);

    ASSERT_EQ(mgr->GetCriterionComputation().getCriterionInputData().Criteria().size(), 2u);
}

// ═══════════════════════════════════════════════════════════
//  UpdateMaxCriterionArea
// ═══════════════════════════════════════════════════════════

TEST_F(BendersOuterLoopManagerTest, UpdateMaxCriterionArea_EmptyCriteria_NoOp)
{
    auto mgr = MakeManager();
    data_.criteria.criteria.clear();
    // Should not crash
    mgr->UpdateMaxCriterionArea();
    ASSERT_EQ(data_.criteria.max_criterion, 0.0);
    ASSERT_EQ(data_.criteria.max_criterion_area, "N/A");
}

TEST_F(BendersOuterLoopManagerTest, UpdateMaxCriterionArea_FindsMaxAndArea)
{
    auto mgr = MakeManager();

    // Set up criterion input data with 3 areas
    Benders::Criterion::CriterionInputData input_data;
    input_data.AddSingleData(
      Benders::Criterion::CriterionSingleInputData("prefix::", "area_north", 100.0));
    input_data.AddSingleData(
      Benders::Criterion::CriterionSingleInputData("prefix::", "area_south", 200.0));
    input_data.AddSingleData(
      Benders::Criterion::CriterionSingleInputData("prefix::", "area_east", 50.0));
    mgr->SetCriterionComputationInputs(input_data);

    // Set criteria values — max is at index 1 (area_south)
    data_.criteria.criteria = {10.0, 30.0, 5.0};

    mgr->UpdateMaxCriterionArea();

    ASSERT_EQ(data_.criteria.max_criterion, 30.0);
    ASSERT_EQ(data_.criteria.max_criterion_area, "area_south");
}

TEST_F(BendersOuterLoopManagerTest, UpdateMaxCriterionArea_SingleCriterion)
{
    auto mgr = MakeManager();

    Benders::Criterion::CriterionInputData input_data;
    input_data.AddSingleData(
      Benders::Criterion::CriterionSingleInputData("prefix::", "only_area", 100.0));
    mgr->SetCriterionComputationInputs(input_data);

    data_.criteria.criteria = {42.0};

    mgr->UpdateMaxCriterionArea();

    ASSERT_EQ(data_.criteria.max_criterion, 42.0);
    ASSERT_EQ(data_.criteria.max_criterion_area, "only_area");
}

TEST_F(BendersOuterLoopManagerTest, UpdateMaxCriterionArea_MaxAtFirstPosition)
{
    auto mgr = MakeManager();

    Benders::Criterion::CriterionInputData input_data;
    input_data.AddSingleData(
      Benders::Criterion::CriterionSingleInputData("prefix::", "area_a", 100.0));
    input_data.AddSingleData(
      Benders::Criterion::CriterionSingleInputData("prefix::", "area_b", 200.0));
    mgr->SetCriterionComputationInputs(input_data);

    data_.criteria.criteria = {99.0, 1.0};

    mgr->UpdateMaxCriterionArea();

    ASSERT_EQ(data_.criteria.max_criterion, 99.0);
    ASSERT_EQ(data_.criteria.max_criterion_area, "area_a");
}

// ═══════════════════════════════════════════════════════════
//  Const accessor for CriterionComputation
// ═══════════════════════════════════════════════════════════

TEST_F(BendersOuterLoopManagerTest, ConstGetCriterionComputation)
{
    auto mgr = MakeManager();

    Benders::Criterion::CriterionInputData input_data;
    input_data.AddSingleData(
      Benders::Criterion::CriterionSingleInputData("prefix::", "area1", 100.0));
    mgr->SetCriterionComputationInputs(input_data);

    const auto& const_mgr = *mgr;
    ASSERT_FALSE(const_mgr.GetCriterionComputation().IsEmpty());
}

} // anonymous namespace
