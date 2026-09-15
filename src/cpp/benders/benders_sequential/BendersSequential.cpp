#include "antares-xpansion/benders/benders_sequential/BendersSequential.h"

#include <algorithm>
#include <iomanip>
#include <utility>

#include "antares-xpansion/benders/benders_core/BendersProblemFromFile.h"
#include "antares-xpansion/benders/benders_core/SequentialCommunicationStrategy.h"
#include "antares-xpansion/helpers/Timer.h"
#include "antares-xpansion/helpers/solver_utils.h"

BendersSequential::BendersSequential(const BendersBaseOptions& options,
                                     Logger logger,
                                     std::shared_ptr<Output::OutputWriter> writer,
                                     std::shared_ptr<MathLoggerDriver> mathLoggerDriver):
    BendersBase(options,
                std::move(logger),
                std::move(writer),
                mathLoggerDriver,
                std::make_shared<SequentialCommunicationStrategy>()),
    cuts_manager_(std::make_shared<BendersCutsManagerSequential>(_data,
                                                                 _problem_to_id,
                                                                 relevantIterationData_,
                                                                 _master)),
    subproblems_manager_(
      std::make_shared<BendersSubProblemsManagerSequential>(_data,
                                                            _options,
                                                            benders_plugin_,
                                                            output_manager_->GetLogger(),
                                                            solver_log_manager_,
                                                            output_manager_->GetWriter(),
                                                            shouldParallelize()))
{
}

void BendersSequential::InitializeProblems()
{
    MatchProblemToId();

    std::vector<SubProblemNamesInCut> subproblem_per_cut_indices;

    if (_data.control.nsubproblem > 0) [[likely]]
    {
        int n_cuts = SetAggregation(_data.control.nsubproblem);
        subproblem_per_cut_indices.reserve(n_cuts);

        SubProblemNamesInCut current_cut;
        size_t group_size = (_data.control.nsubproblem + n_cuts - 1) / n_cuts;
        current_cut.reserve(group_size);

        for (const auto& [name, id]: _problem_to_id)
        {
            current_cut.emplace_back(name, 0);
            if (current_cut.size() == group_size)
            {
                subproblem_per_cut_indices.emplace_back(std::move(current_cut));
                current_cut.clear();
                current_cut.reserve(group_size);
            }
        }
        if (!current_cut.empty())
        {
            subproblem_per_cut_indices.emplace_back(std::move(current_cut));
        }
    }
    cuts_manager_->SetSubproblemPerCutIndices(std::move(subproblem_per_cut_indices));

    std::shared_ptr<IBendersProblemProvider>
      benders_problem_provider = std::make_shared<BendersProblemFromFile>(get_master_path());
    reset_master(master_variable_map_,
                 _options.SOLVER_NAME,
                 _options.LOG_LEVEL,
                 _data.control.nsubproblem,
                 solver_log_manager_,
                 IsResumeMode(),
                 output_manager_->GetLogger(),
                 Options().PROBLEMS_FORMAT,
                 benders_problem_provider.get(),
                 Options().MASTER_SOLUTION_TOLERANCE,
                 GetSubCutTolerance());
    subproblems_manager_->SetCouplingMap(coupling_map_);
    for (const auto& problem: coupling_map_)
    {
        subproblems_manager_->AddSubproblem(problem);
        subproblems_manager_->AddSubproblemName(problem.first);
    }
    subproblems_manager_->BuildSubproblemWorkerFactory(_options.CACHE_PROBLEMS);
}

void BendersSequential::free()
{
    if (get_master())
    {
        master_manager_->FreeMaster();
    }
    subproblems_manager_->free_subproblems();
}

void BendersSequential::BuildCut()
{
    SubProblemDataMap subproblem_data_map;
    Timer timer;
    subproblems_manager_->GetSubproblemCut(subproblem_data_map,
                                           subproblems_manager_->MakeFastBeginHook(),
                                           subproblems_manager_->MakeCacheBeginHook(),
                                           subproblems_manager_->MakePostSolveHook());
    _data.cuts.subproblem_cost = 0;
    for (const auto& [_, subproblem_data]: subproblem_data_map)
    {
        _data.cuts.subproblem_cost += subproblem_data.subproblem_cost;
    }

    _data.cuts.subproblems_walltime = timer.elapsed();
    check_status(subproblem_data_map);
    cuts_manager_->GatherAndBuildCuts(subproblem_data_map);
}

void BendersSequential::Run()
{
    auto logger = output_manager_->GetLogger();

    init_data();
    ChecksResumeMode();
    if (_options.TRACE)
    {
        output_manager_->OpenCsvFile();
    }

    HandleInitialMasterRelaxation();

    while (!_data.control.stop)
    {
        Timer timer_master;
        ++_data.control.it;

        if (SwitchToIntegerMaster(_data.control.is_in_initial_relaxation))
        {
            logger->LogAtSwitchToInteger();
            ActivateIntegrityConstraints();
            ResetDataPostRelaxation();
        }

        logger->log_at_initialization(_data.control.it
                                      + output_manager_->GetNumIterationsBeforeRestart());
        logger->display_message("\tSolving master...");
        get_master_value();
        logger->log_master_solving_duration(_data.master.timer_master);

        cuts_manager_->ComputeXCut(_data,
                                   Options().SEPARATION_PARAM,
                                   Options().MASTER_SOLUTION_TOLERANCE);
        logger->log_iteration_candidates(output_manager_->bendersDataToLogData(_data));

        logger->display_message("\tSolving subproblems...");
        BuildCut();
        logger->LogSubproblemsSolvingWalltime(_data.cuts.subproblems_walltime);

        compute_ub();
        update_best_ub();

        logger->log_at_iteration_end(output_manager_->bendersDataToLogData(_data));

        UpdateTrace();

        _data.master.timer_master = timer_master.elapsed();
        _data.control.iteration_time = -_data.control.benders_time;
        _data.control.benders_time = GetBendersTime();
        _data.control.iteration_time += _data.control.benders_time;
        _data.control.stop = ShouldBendersStop();
        output_manager_->SaveCurrentBendersData(LastIterationFile(), _options.TRACE);
    }
    output_manager_->CloseCsvFile();
    output_manager_->EndWritingInOutputFile(_data.control.benders_time,
                                            _options.EXTERNAL_LOOP_OPTIONS.DO_OUTER_LOOP);
    write_basis();
}

void BendersSequential::launch()
{
    auto logger = output_manager_->GetLogger();

    logger->display_message("Building input");
    logger->display_message("Constructing workers...");

    InitializeProblems();

    benders_plugin_->OnBendersStart(subproblems_manager_->GetSubProblemMap(),
                                    logger,
                                    _options,
                                    solver_log_manager_,
                                    subproblems_manager_->GetFactorySolver());

    logger->display_message("Running solver...");
    try
    {
        Run();
        logger->display_message(BendersName() + " solver terminated.");
    }
    catch (const std::exception& ex)
    {
        std::string error = "Exception raised : " + std::string(ex.what());
        logger->display_message(error);
    }

    post_run_actions();
    free();
}
