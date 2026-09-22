#include "antares-xpansion/benders/benders_core/BendersOutputManager.h"

#include <filesystem>
#include <memory>

#include "LoggerStub.h"
#include "WriterStub.h"
#include "antares-xpansion/benders/benders_core/BendersMathLogger.h"
#include "antares-xpansion/benders/output/OutputWriter.h"
#include "gtest/gtest.h"

namespace
{

// ─── Recording writer: captures calls for assertions ───
class RecordingWriter : public Xpansion::Test::WriterNOOPStub
{
public:
    int write_solution_calls = 0;
    int write_iteration_calls = 0;
    int write_nbweeks_calls = 0;
    int dump_calls = 0;
    int update_end_time_calls = 0;
    int write_duration_calls = 0;

    Output::SolutionData last_solution;
    size_t last_iteration_num = 0;
    int last_nb_weeks = 0;
    double last_duration = 0.0;

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

    void updateEndTime() override { ++update_end_time_calls; }

    void write_duration(const double duration) override
    {
        ++write_duration_calls;
        last_duration = duration;
    }

    void write_nbweeks(const int nb_weeks) override
    {
        ++write_nbweeks_calls;
        last_nb_weeks = nb_weeks;
    }
};

// ─── Recording logger: captures calls for assertions ───
class RecordingLogger : public Xpansion::Test::LoggerNOOPStub
{
public:
    int log_stop_criterion_calls = 0;
    int log_at_ending_calls = 0;
    StoppingCriterion last_stopping_criterion = StoppingCriterion::empty;
    LogData last_ending_data;

    void log_stop_criterion_reached(const StoppingCriterion stopping_criterion) override
    {
        ++log_stop_criterion_calls;
        last_stopping_criterion = stopping_criterion;
    }

    void log_at_ending(const LogData& d) override
    {
        ++log_at_ending_calls;
        last_ending_data = d;
    }
};

// Helper to build a default BendersBaseOptions
BendersBaseOptions MakeDefaultOptions()
{
    SolverBaseOptions base;
    BendersBaseOptions opts(base);
    opts.OUTPUTROOT = std::filesystem::temp_directory_path().string();
    opts.CSV_NAME = "test_trace";
    return opts;
}

// ─── Fixture ───
class BendersOutputManagerTest : public ::testing::Test
{
public:
    CurrentIterationData data_;
    BendersBaseOptions options_{MakeDefaultOptions()};
    VariableMap problem_to_id_;
    BendersRelevantIterationsData relevant_data_;

    std::shared_ptr<RecordingWriter> writer_;
    std::shared_ptr<RecordingLogger> recording_logger_;
    Logger logger_;
    std::shared_ptr<MathLoggerDriver> math_logger_;

    void SetUp() override
    {
        data_ = {};
        problem_to_id_ = {};
        relevant_data_ = {};

        writer_ = std::make_shared<RecordingWriter>();
        recording_logger_ = std::make_shared<RecordingLogger>();
        logger_ = recording_logger_;
        math_logger_ = std::make_shared<MathLoggerDriver>();
    }

    std::shared_ptr<BendersOutputManager> MakeManager()
    {
        return std::make_shared<BendersOutputManager>(logger_, writer_, math_logger_, data_,
                                                      options_, problem_to_id_, relevant_data_);
    }

    // Helper to set up a valid WorkerMasterData with shared_ptr Points
    void SetupValidWorkerMasterData(WorkerMasterData& wmd)
    {
        wmd._valid = true;
        wmd._lb = 100.0;
        wmd._ub = 200.0;
        wmd._best_ub = 150.0;
        wmd._master_duration = 1.0;
        wmd._subproblem_duration = 2.0;
        wmd._invest_cost = 50.0;
        wmd._operational_cost = 100.0;
        wmd._x_in = std::make_shared<Point>(Point{{"cand1", 10.0}});
        wmd._x_out = std::make_shared<Point>(Point{{"cand1", 12.0}});
        wmd._x_cut = std::make_shared<Point>(Point{{"cand1", 11.0}});
        wmd._min_invest = std::make_shared<Point>(Point{{"cand1", 0.0}});
        wmd._max_invest = std::make_shared<Point>(Point{{"cand1", 100.0}});
    }
};

// ═══════════════════════════════════════════════════════════
//  Getters
// ═══════════════════════════════════════════════════════════

TEST_F(BendersOutputManagerTest, GetLogger_ReturnsSameLogger)
{
    auto mgr = MakeManager();
    ASSERT_EQ(mgr->GetLogger(), logger_);
}

TEST_F(BendersOutputManagerTest, GetWriter_ReturnsSameWriter)
{
    auto mgr = MakeManager();
    ASSERT_EQ(mgr->GetWriter(), writer_);
}

TEST_F(BendersOutputManagerTest, GetMathLoggerDriver_ReturnsSameDriver)
{
    auto mgr = MakeManager();
    ASSERT_EQ(mgr->GetMathLoggerDriver(), math_logger_);
}

// ═══════════════════════════════════════════════════════════
//  WriteNbWeeks
// ═══════════════════════════════════════════════════════════

TEST_F(BendersOutputManagerTest, WriteNbWeeks_DelegatesToWriter)
{
    auto mgr = MakeManager();
    mgr->WriteNbWeeks(52);

    ASSERT_EQ(writer_->write_nbweeks_calls, 1);
    ASSERT_EQ(writer_->last_nb_weeks, 52);
}

// ═══════════════════════════════════════════════════════════
//  bendersDataToLogData
// ═══════════════════════════════════════════════════════════

TEST_F(BendersOutputManagerTest, BendersDataToLogData_MapsFieldsCorrectly)
{
    auto mgr = MakeManager();

    data_.master.lb = 100.0;
    data_.control.best_ub = 200.0;
    data_.cuts.ub = 250.0;
    data_.control.it = 5;
    data_.control.best_it = 3;
    data_.cuts.subproblem_cost = 80.0;
    data_.master.invest_cost = 60.0;
    data_.solution.x_in = {{"a", 1.0}};
    data_.solution.x_out = {{"a", 2.0}};
    data_.solution.x_cut = {{"a", 1.5}};
    data_.solution.min_invest = {{"a", 0.0}};
    data_.solution.max_invest = {{"a", 100.0}};
    data_.control.benders_time = 10.0;
    data_.master.timer_master = 3.0;
    data_.cuts.subproblems_walltime = 7.0;
    data_.control.cumulative_number_of_subproblem_solved = 20;
    options_.MAX_ITERATIONS = 100;

    auto log = mgr->bendersDataToLogData(data_);

    ASSERT_EQ(log.lb, 100.0);
    ASSERT_EQ(log.best_ub, 200.0);
    ASSERT_EQ(log.ub, 250.0);
    ASSERT_EQ(log.it, 5);
    ASSERT_EQ(log.best_it, 3);
    ASSERT_EQ(log.subproblem_cost, 80.0);
    ASSERT_EQ(log.invest_cost, 60.0);
    ASSERT_EQ(log.x_in.at("a"), 1.0);
    ASSERT_EQ(log.x_out.at("a"), 2.0);
    ASSERT_EQ(log.x_cut.at("a"), 1.5);
    ASSERT_EQ(log.min_invest.at("a"), 0.0);
    ASSERT_EQ(log.max_invest.at("a"), 100.0);
    ASSERT_DOUBLE_EQ(log.optimality_gap, 100.0);   // 200 - 100
    ASSERT_DOUBLE_EQ(log.relative_gap, 0.5);       // 100 / 200
    ASSERT_EQ(log.max_iterations, 100);
    ASSERT_EQ(log.benders_elapsed_time, 10.0);
    ASSERT_EQ(log.master_time, 3.0);
    ASSERT_EQ(log.subproblem_time, 7.0);
    ASSERT_EQ(log.cumulative_number_of_subproblem_resolved, 20);
}

TEST_F(BendersOutputManagerTest, BendersDataToLogData_AccountsForResumeOffsets)
{
    auto mgr = MakeManager();
    mgr->SetIterationsBeforeResume(10);
    mgr->SetCumulativeSubproblemsSolvedBeforeResume(50);

    data_.control.it = 3;
    data_.control.best_it = 2;
    data_.control.best_ub = 200.0;
    data_.master.lb = 100.0;
    data_.control.cumulative_number_of_subproblem_solved = 5;

    auto log = mgr->bendersDataToLogData(data_);

    ASSERT_EQ(log.it, 13);       // 3 + 10
    ASSERT_EQ(log.best_it, 12);  // 2 + 10
    ASSERT_EQ(log.cumulative_number_of_subproblem_resolved, 55); // 5 + 50
}

// ═══════════════════════════════════════════════════════════
//  iteration (WorkerMasterData -> Output::Iteration)
// ═══════════════════════════════════════════════════════════

TEST_F(BendersOutputManagerTest, Iteration_ConvertsWorkerMasterData)
{
    auto mgr = MakeManager();

    WorkerMasterData wmd;
    SetupValidWorkerMasterData(wmd);
    data_.control.cumulative_number_of_subproblem_solved = 15;

    auto iter = mgr->iteration(wmd);

    ASSERT_EQ(iter.master_duration, 1.0);
    ASSERT_EQ(iter.subproblem_duration, 2.0);
    ASSERT_EQ(iter.lb, 100.0);
    ASSERT_EQ(iter.ub, 200.0);
    ASSERT_EQ(iter.best_ub, 150.0);
    ASSERT_DOUBLE_EQ(iter.optimality_gap, 50.0);        // 150 - 100
    ASSERT_DOUBLE_EQ(iter.relative_gap, 50.0 / 150.0);  // gap / best_ub
    ASSERT_EQ(iter.investment_cost, 50.0);
    ASSERT_EQ(iter.operational_cost, 100.0);
    ASSERT_EQ(iter.overall_cost, 150.0);
    ASSERT_EQ(iter.cumulative_number_of_subproblem_resolved, 15);
    ASSERT_EQ(iter.candidates.size(), 1u);
    ASSERT_EQ(iter.candidates[0].name, "cand1");
    ASSERT_EQ(iter.candidates[0].invest, 11.0);
    ASSERT_EQ(iter.candidates[0].min, 0.0);
    ASSERT_EQ(iter.candidates[0].max, 100.0);
}

TEST_F(BendersOutputManagerTest, Iteration_IncludesResumeOffset)
{
    auto mgr = MakeManager();
    mgr->SetCumulativeSubproblemsSolvedBeforeResume(100);

    WorkerMasterData wmd;
    SetupValidWorkerMasterData(wmd);
    data_.control.cumulative_number_of_subproblem_solved = 5;

    auto iter = mgr->iteration(wmd);
    ASSERT_EQ(iter.cumulative_number_of_subproblem_resolved, 105);
}

// ═══════════════════════════════════════════════════════════
//  BuildSolution
// ═══════════════════════════════════════════════════════════

TEST_F(BendersOutputManagerTest, BuildSolution_NonResume_UsesBestWorkerMasterData)
{
    options_.RESUME = false;
    auto mgr = MakeManager();

    SetupValidWorkerMasterData(relevant_data_.best);
    data_.control.best_it = 3;
    data_.control.best_ub = 150.0;
    data_.master.lb = 100.0;
    data_.control.stopping_criterion = StoppingCriterion::relative_gap;

    auto sol = mgr->BuildSolution(52);

    ASSERT_EQ(sol.nbWeeks_p, 52);
    ASSERT_EQ(sol.best_it, 3);  // best_it + 0 resume offset
    ASSERT_EQ(sol.problem_status, "OPTIMAL");
    ASSERT_EQ(sol.stopping_criterion, "relative gap");
}

TEST_F(BendersOutputManagerTest, BuildSolution_ResumeMode_UsesBestIterationData)
{
    options_.RESUME = true;
    auto mgr = MakeManager();
    mgr->SetIterationsBeforeResume(10);

    // Set up best iteration data
    LogData best;
    best.lb = 90.0;
    best.best_ub = 140.0;
    best.invest_cost = 40.0;
    best.subproblem_cost = 70.0;
    best.master_time = 2.0;
    best.subproblem_time = 5.0;
    best.ub = 180.0;
    best.x_cut = {{"cand1", 15.0}};
    best.min_invest = {{"cand1", 0.0}};
    best.max_invest = {{"cand1", 100.0}};
    mgr->UpdateBestIterationData(best);

    data_.control.best_it = 2;
    data_.control.best_ub = 140.0;
    data_.master.lb = 90.0;
    data_.control.stopping_criterion = StoppingCriterion::absolute_gap;

    auto sol = mgr->BuildSolution(52);

    ASSERT_EQ(sol.best_it, 12); // 2 + 10
    ASSERT_EQ(sol.problem_status, "OPTIMAL");
    ASSERT_EQ(sol.solution.investment_cost, 40.0);
    ASSERT_EQ(sol.solution.candidates.size(), 1u);
    ASSERT_EQ(sol.solution.candidates[0].name, "cand1");
}

// ═══════════════════════════════════════════════════════════
//  status_from_criterion (tested through BuildSolution)
// ═══════════════════════════════════════════════════════════

TEST_F(BendersOutputManagerTest, StatusFromCriterion_AbsoluteGap_IsOptimal)
{
    auto mgr = MakeManager();
    SetupValidWorkerMasterData(relevant_data_.best);
    data_.control.stopping_criterion = StoppingCriterion::absolute_gap;
    data_.control.best_ub = 100.0;
    data_.master.lb = 90.0;

    auto sol = mgr->BuildSolution(0);
    ASSERT_EQ(sol.problem_status, "OPTIMAL");
}

TEST_F(BendersOutputManagerTest, StatusFromCriterion_MaxIteration_IsLimitReached)
{
    auto mgr = MakeManager();
    SetupValidWorkerMasterData(relevant_data_.best);
    data_.control.stopping_criterion = StoppingCriterion::max_iteration;
    data_.control.best_ub = 100.0;
    data_.master.lb = 90.0;

    auto sol = mgr->BuildSolution(0);
    ASSERT_EQ(sol.problem_status, "limit reached");
}

TEST_F(BendersOutputManagerTest, StatusFromCriterion_Timelimit_IsLimitReached)
{
    auto mgr = MakeManager();
    SetupValidWorkerMasterData(relevant_data_.best);
    data_.control.stopping_criterion = StoppingCriterion::timelimit;
    data_.control.best_ub = 100.0;
    data_.master.lb = 90.0;

    auto sol = mgr->BuildSolution(0);
    ASSERT_EQ(sol.problem_status, "limit reached");
}

TEST_F(BendersOutputManagerTest, StatusFromCriterion_Empty_IsError)
{
    auto mgr = MakeManager();
    SetupValidWorkerMasterData(relevant_data_.best);
    data_.control.stopping_criterion = StoppingCriterion::empty;
    data_.control.best_ub = 100.0;
    data_.master.lb = 90.0;

    auto sol = mgr->BuildSolution(0);
    ASSERT_EQ(sol.problem_status, "ERROR");
}

// ═══════════════════════════════════════════════════════════
//  EndWritingInOutputFile
// ═══════════════════════════════════════════════════════════

TEST_F(BendersOutputManagerTest, EndWriting_WithoutOuterLoop_WritesSolution)
{
    auto mgr = MakeManager();
    SetupValidWorkerMasterData(relevant_data_.best);
    data_.control.best_ub = 100.0;
    data_.master.lb = 90.0;

    mgr->EndWritingInOutputFile(42.0, false);

    ASSERT_EQ(writer_->update_end_time_calls, 1);
    ASSERT_EQ(writer_->write_duration_calls, 1);
    ASSERT_EQ(writer_->last_duration, 42.0);
    ASSERT_EQ(writer_->write_solution_calls, 1);
    ASSERT_EQ(writer_->dump_calls, 1);
}

TEST_F(BendersOutputManagerTest, EndWriting_WithOuterLoop_SkipsSolution)
{
    auto mgr = MakeManager();

    mgr->EndWritingInOutputFile(42.0, true);

    ASSERT_EQ(writer_->update_end_time_calls, 1);
    ASSERT_EQ(writer_->write_duration_calls, 1);
    ASSERT_EQ(writer_->write_solution_calls, 0);
}

// ═══════════════════════════════════════════════════════════
//  PostRunActions
// ═══════════════════════════════════════════════════════════

TEST_F(BendersOutputManagerTest, PostRunActions_LogsStopCriterionAndEnding)
{
    auto mgr = MakeManager();
    data_.control.it = 10;
    data_.control.best_it = 7;
    options_.ABSOLUTE_GAP = 0.01;
    options_.RELATIVE_GAP = 0.05;
    options_.MAX_ITERATIONS = 100;

    mgr->PostRunActions(StoppingCriterion::relative_gap);

    ASSERT_EQ(recording_logger_->log_stop_criterion_calls, 1);
    ASSERT_EQ(recording_logger_->last_stopping_criterion, StoppingCriterion::relative_gap);
    ASSERT_EQ(recording_logger_->log_at_ending_calls, 1);
    ASSERT_EQ(recording_logger_->last_ending_data.it, 10);
    ASSERT_EQ(recording_logger_->last_ending_data.best_it, 7);
    ASSERT_DOUBLE_EQ(recording_logger_->last_ending_data.optimality_gap, 0.01);
    ASSERT_DOUBLE_EQ(recording_logger_->last_ending_data.relative_gap, 0.05);
    ASSERT_EQ(recording_logger_->last_ending_data.max_iterations, 100);
}

// ═══════════════════════════════════════════════════════════
//  BuildFinalLogData
// ═══════════════════════════════════════════════════════════

TEST_F(BendersOutputManagerTest, BuildFinalLogData_UsesOptionsAndBestIteration)
{
    auto mgr = MakeManager();

    LogData best;
    best.subproblem_cost = 80.0;
    best.invest_cost = 40.0;
    mgr->UpdateBestIterationData(best);

    data_.control.it = 15;
    data_.control.best_it = 10;
    data_.control.cumulative_number_of_subproblem_solved = 30;
    options_.ABSOLUTE_GAP = 0.1;
    options_.RELATIVE_GAP = 0.05;
    options_.MAX_ITERATIONS = 200;

    auto log = mgr->BuildFinalLogData();

    ASSERT_EQ(log.it, 15);
    ASSERT_EQ(log.best_it, 10);
    ASSERT_EQ(log.subproblem_cost, 80.0);
    ASSERT_EQ(log.invest_cost, 40.0);
    ASSERT_DOUBLE_EQ(log.optimality_gap, 0.1);
    ASSERT_DOUBLE_EQ(log.relative_gap, 0.05);
    ASSERT_EQ(log.max_iterations, 200);
    ASSERT_EQ(log.cumulative_number_of_subproblem_resolved, 30);
}

TEST_F(BendersOutputManagerTest, BuildFinalLogData_IncludesResumeOffsets)
{
    auto mgr = MakeManager();
    mgr->SetIterationsBeforeResume(5);
    mgr->SetCumulativeSubproblemsSolvedBeforeResume(20);

    data_.control.it = 10;
    data_.control.best_it = 7;
    data_.control.cumulative_number_of_subproblem_solved = 30;

    auto log = mgr->BuildFinalLogData();

    ASSERT_EQ(log.it, 15);        // 10 + 5
    ASSERT_EQ(log.best_it, 12);   // 7 + 5
    ASSERT_EQ(log.cumulative_number_of_subproblem_resolved, 50); // 30 + 20
}

// ═══════════════════════════════════════════════════════════
//  Resume support: getters/setters
// ═══════════════════════════════════════════════════════════

TEST_F(BendersOutputManagerTest, ResumeGettersSetters_DefaultValues)
{
    auto mgr = MakeManager();
    ASSERT_EQ(mgr->GetNumIterationsBeforeRestart(), 0);
    ASSERT_EQ(mgr->GetNumOfSubProblemsSolvedBeforeResume(), 0);
}

TEST_F(BendersOutputManagerTest, ResumeGettersSetters_SetAndGet)
{
    auto mgr = MakeManager();
    mgr->SetIterationsBeforeResume(42);
    mgr->SetCumulativeSubproblemsSolvedBeforeResume(99);

    ASSERT_EQ(mgr->GetNumIterationsBeforeRestart(), 42);
    ASSERT_EQ(mgr->GetNumOfSubProblemsSolvedBeforeResume(), 99);
}

// ═══════════════════════════════════════════════════════════
//  UpdateBestIterationData / GetBestIterationData
// ═══════════════════════════════════════════════════════════

TEST_F(BendersOutputManagerTest, BestIterationData_UpdateAndGet)
{
    auto mgr = MakeManager();

    LogData best;
    best.lb = 100.0;
    best.best_ub = 150.0;
    best.it = 5;
    best.invest_cost = 40.0;
    mgr->UpdateBestIterationData(best);

    const auto& retrieved = mgr->GetBestIterationData();
    ASSERT_EQ(retrieved.lb, 100.0);
    ASSERT_EQ(retrieved.best_ub, 150.0);
    ASSERT_EQ(retrieved.it, 5);
    ASSERT_EQ(retrieved.invest_cost, 40.0);
}

TEST_F(BendersOutputManagerTest, BestIterationData_OverwritesPrevious)
{
    auto mgr = MakeManager();

    LogData first;
    first.lb = 50.0;
    mgr->UpdateBestIterationData(first);

    LogData second;
    second.lb = 90.0;
    mgr->UpdateBestIterationData(second);

    ASSERT_EQ(mgr->GetBestIterationData().lb, 90.0);
}

// ═══════════════════════════════════════════════════════════
//  CSV file open/close
// ═══════════════════════════════════════════════════════════

TEST_F(BendersOutputManagerTest, OpenAndCloseCsvFile_NoResume_WritesHeader)
{
    options_.RESUME = false;
    auto mgr = MakeManager();

    mgr->OpenCsvFile();
    mgr->CloseCsvFile();

    // Verify the file was created and has a header
    std::ifstream check(std::filesystem::path(options_.OUTPUTROOT) / "test_trace.csv");
    ASSERT_TRUE(check.is_open());
    std::string header;
    std::getline(check, header);
    ASSERT_FALSE(header.empty());
    check.close();

    // Clean up
    std::filesystem::remove(std::filesystem::path(options_.OUTPUTROOT) / "test_trace.csv");
}

TEST_F(BendersOutputManagerTest, OpenCsvFile_CalledTwice_DoesNotReopen)
{
    options_.RESUME = false;
    auto mgr = MakeManager();

    mgr->OpenCsvFile();
    mgr->OpenCsvFile(); // should be no-op
    mgr->CloseCsvFile();

    // Clean up
    std::filesystem::remove(std::filesystem::path(options_.OUTPUTROOT) / "test_trace.csv");
}

TEST_F(BendersOutputManagerTest, CloseCsvFile_WhenNotOpen_NoOp)
{
    auto mgr = MakeManager();
    // Should not crash
    mgr->CloseCsvFile();
}

// ═══════════════════════════════════════════════════════════
//  MathLogger delegation
// ═══════════════════════════════════════════════════════════

TEST_F(BendersOutputManagerTest, MathLoggerPrint_DoesNotCrash)
{
    auto mgr = MakeManager();
    // MathLoggerDriver with no loggers added - should just no-op
    mgr->MathLoggerPrint();
}

TEST_F(BendersOutputManagerTest, MathLoggerWriteHeader_DoesNotCrash)
{
    auto mgr = MakeManager();
    mgr->MathLoggerWriteHeader();
}

} // anonymous namespace
