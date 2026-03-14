#include "antares-xpansion/benders/benders_core/BendersCore.h"

#include <memory>

#include "antares-xpansion/benders/benders_core/BendersProblemFromFile.h"
#include "antares-xpansion/benders/benders_core/strategies/MPISubproblemSolver.h"
#include "antares-xpansion/benders/benders_core/strategies/SingleLoopStrategy.h"
#include "antares-xpansion/benders/benders_core/strategies/StandardBatchStrategy.h"
#include "antares-xpansion/helpers/Timer.h"

BendersCore::BendersCore(BendersBaseOptions options,
                         Logger logger,
                         std::shared_ptr<Output::OutputWriter> writer,
                         std::shared_ptr<MathLoggerDriver> mathLoggerDriver):
    BendersBase(std::move(options), std::move(logger), std::move(writer), mathLoggerDriver),
    solver_strategy_(std::make_unique<MPISubproblemSolver>()),
    loop_strategy_(std::make_unique<SingleLoopStrategy>()),
    batch_strategy_(std::make_unique<StandardBatchStrategy>()),
    world_(nullptr)
{
}

BendersCore::BendersCore(BendersBaseOptions options,
                         Logger logger,
                         std::shared_ptr<Output::OutputWriter> writer,
                         std::shared_ptr<MathLoggerDriver> mathLoggerDriver,
                         mpi::communicator* world):
    BendersBase(std::move(options), std::move(logger), std::move(writer), mathLoggerDriver),
    solver_strategy_(std::make_unique<MPISubproblemSolver>()),
    loop_strategy_(std::make_unique<SingleLoopStrategy>()),
    batch_strategy_(std::make_unique<StandardBatchStrategy>()),
    world_(world)
{
}

BendersCore::BendersCore(BendersBaseOptions options,
                         Logger logger,
                         std::shared_ptr<Output::OutputWriter> writer,
                         std::shared_ptr<MathLoggerDriver> mathLoggerDriver,
                         SubproblemSolverPtr solver_strategy,
                         LoopStrategyPtr loop_strategy):
    BendersBase(std::move(options), std::move(logger), std::move(writer), mathLoggerDriver),
    solver_strategy_(std::move(solver_strategy)),
    loop_strategy_(std::move(loop_strategy)),
    batch_strategy_(std::make_unique<StandardBatchStrategy>()),
    world_(nullptr)
{
}

BendersCore::BendersCore(BendersBaseOptions options,
                         Logger logger,
                         std::shared_ptr<Output::OutputWriter> writer,
                         std::shared_ptr<MathLoggerDriver> mathLoggerDriver,
                         SubproblemSolverPtr solver_strategy,
                         LoopStrategyPtr loop_strategy,
                         BatchStrategyPtr batch_strategy):
    BendersBase(std::move(options), std::move(logger), std::move(writer), mathLoggerDriver),
    solver_strategy_(std::move(solver_strategy)),
    loop_strategy_(std::move(loop_strategy)),
    batch_strategy_(std::move(batch_strategy)),
    world_(nullptr)
{
}

BendersCore::BendersCore(BendersBaseOptions options,
                         Logger logger,
                         std::shared_ptr<Output::OutputWriter> writer,
                         std::shared_ptr<MathLoggerDriver> mathLoggerDriver,
                         mpi::communicator* world,
                         SubproblemSolverPtr solver_strategy,
                         LoopStrategyPtr loop_strategy,
                         BatchStrategyPtr batch_strategy):
    BendersBase(std::move(options), std::move(logger), std::move(writer), mathLoggerDriver),
    solver_strategy_(std::move(solver_strategy)),
    loop_strategy_(std::move(loop_strategy)),
    batch_strategy_(std::move(batch_strategy)),
    world_(world)
{
}

int BendersCore::Rank() const
{
    return world_ ? world_->rank() : 0;
}

int BendersCore::WorldSize() const
{
    return world_ ? world_->size() : 1;
}

void BendersCore::launch()
{
    _logger->display_message("Building input");
    _logger->display_message("Constructing workers...");

    InitializeProblems();
    _logger->display_message("Running solver...");
    try
    {
        Run();
        _logger->display_message(BendersName() + " solver terminated.");
    }
    catch (const std::exception& ex)
    {
        std::string error = "Exception raised : " + std::string(ex.what());
        _logger->display_message(error);
    }

    post_run_actions();
    free();
}

std::string BendersCore::BendersName() const
{
    return "BendersCore";
}

void BendersCore::InitializeProblems()
{
    MatchProblemToId();
    std::shared_ptr<IBendersProblemProvider>
      benders_problem_provider = std::make_shared<BendersProblemFromFile>(get_master_path());
    reset_master<WorkerMaster>(master_variable_map_,
                               get_solver_name(),
                               get_log_level(),
                               _data.nsubproblem,
                               solver_log_manager_,
                               IsResumeMode(),
                               _logger,
                               Options().PROBLEMS_FORMAT,
                               benders_problem_provider.get(),
                               Options().MASTER_SOLUTION_TOLERANCE,
                               Options().CUT_COEFFICIENT_TOLERANCE);
    for (const auto& problem: coupling_map_)
    {
        const auto subProblemFilePath = GetSubproblemPath(problem.first);

        AddSubproblem(problem);
        AddSubproblemName(problem.first);
    }
}

void BendersCore::Run()
{
    if (batch_strategy_)
    {
        batch_strategy_->Run(*this);
    }
    else
    {
        RunCore();
    }
}

void BendersCore::RunCore()
{
    init_data();
    ChecksResumeMode();
    if (is_trace())
    {
        OpenCsvFile();
    }

    HandleInitialMasterRelaxation();

    while (!_data.stop)
    {
        Timer timer_master;
        ++_data.it;

        if (SwitchToIntegerMaster(_data.is_in_initial_relaxation))
        {
            _logger->LogAtSwitchToInteger();
            ActivateIntegrityConstraints();
            ResetDataPostRelaxation();
        }

        _logger->log_at_initialization(_data.it + GetNumIterationsBeforeRestart());
        _logger->display_message("\tSolving master...");
        get_master_value();
        _logger->log_master_solving_duration(_data.timer_master);

        ComputeXCut();
        _logger->log_iteration_candidates(bendersDataToLogData(_data));

        _logger->display_message("\tSolving subproblems...");
        BuildCut();
        _logger->LogSubproblemsSolvingWalltime(_data.subproblems_walltime);

        compute_ub();
        update_best_ub();

        _logger->log_at_iteration_end(bendersDataToLogData(_data));

        UpdateTrace();

        _data.timer_master = timer_master.elapsed();
        _data.iteration_time = -_data.benders_time;
        _data.benders_time = GetBendersTime();
        _data.iteration_time += _data.benders_time;
        _data.stop = ShouldBendersStop();
        SaveCurrentBendersData();
    }
    CloseCsvFile();
    EndWritingInOutputFile();
    write_basis();
}

void BendersCore::post_run_actions() const
{
    if (_data.best_it != 0)
    {
        SaveSolutionInOutputFile();
    }
}

void BendersCore::BuildCut()
{
    SubProblemDataMap subproblem_data_map;
    Timer timer;
    GetSubproblemCut(subproblem_data_map);
    SetSubproblemCost(0);
    for (const auto& [_, subproblem_data]: subproblem_data_map)
    {
        SetSubproblemCost(GetSubproblemCost() + subproblem_data.subproblem_cost);
    }

    _data.subproblems_walltime = timer.elapsed();
    _data.ub = 0;
    BuildCutFull(subproblem_data_map);
}

void BendersCore::free()
{
    if (get_master())
    {
        free_master();
    }
    free_subproblems();
}

bool BendersCore::shouldParallelize() const
{
    return solver_strategy_->should_parallelize();
}
