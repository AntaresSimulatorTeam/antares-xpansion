

#include "antares-xpansion/benders/benders_mpi/BendersMPI.h"

#include <fstream>
#include <utility>

#include "antares-xpansion/benders/benders_core/BendersProblemFromFile.h"
#include "antares-xpansion/benders/benders_core/CriterionComputation.h"
#include "antares-xpansion/benders/benders_core/CustomVector.h"
#include "antares-xpansion/helpers/Timer.h"

BendersMpi::BendersMpi(const BendersBaseOptions& options,
                       std::shared_ptr<ILogger> logger,
                       std::shared_ptr<Output::OutputWriter> writer,
                       mpi::communicator& world,
                       std::shared_ptr<MathLoggerDriver> mathLoggerDriver):
    BendersBase(options,
                logger,
                std::move(writer),
                std::move(mathLoggerDriver),
                std::make_shared<MpiCommunicationStrategy>(world)),
    _world(world),
    cuts_manager_(std::make_shared<BendersCutsManagerMpi>(world,
                                                          rank_0,
                                                          _data,
                                                          _problem_to_id,
                                                          relevantIterationData_,
                                                          _master,
                                                          subproblem_per_cut_indices_)),
    subproblems_manager_(
      std::make_shared<BendersSubProblemsManagerMpi>(_data,
                                                     _options,
                                                     benders_plugin_,
                                                     output_manager_->GetLogger(),
                                                     solver_log_manager_,
                                                     output_manager_->GetWriter(),
                                                     shouldParallelize()))
{
    subproblems_manager_->SetOnVariablesIndicesSet(
      [this](const std::vector<std::string>& col_names)
      { outer_loop_manager_->GetCriterionComputation().SearchVariables(col_names); });
}

void BendersMpi::InitializeProblems()
{
    MatchProblemToId();
    SubProblemNamesInCut subs_per_proc;
    if (_options.CACHE_PROBLEMS > 0)
    {
        int current_problem_id = 0;
        for (auto it = coupling_map_.begin(); it != coupling_map_.end();)
        {
            auto process_to_feed = current_problem_id % _world.size();
            if (process_to_feed != _world.rank())
            {
                it = coupling_map_.erase(it);
            }
            else
            {
                subproblems_manager_->AddSubproblemName(it->first);
                subs_per_proc.emplace_back(it->first, process_to_feed);
                ++it;
            }
            current_problem_id++;
        }
    }
    else
    {
        int current_problem_id = 0;
        for (const auto& problem: coupling_map_)
        {
            if (auto process_to_feed = current_problem_id % _world.size();
                process_to_feed == _world.rank())
            {
                subs_per_proc.push_back(std::make_pair(problem.first, process_to_feed));
                subproblems_manager_->AddSubproblem(problem);
                subproblems_manager_->AddSubproblemName(problem.first);
            }
            current_problem_id++;
        }
    }
    subproblems_manager_->SetCouplingMap(coupling_map_);

    std::vector<SubProblemNamesInCut> gathered_subs_per_proc;
    mpi::gather(_world, subs_per_proc, gathered_subs_per_proc, rank_0);
    if (_world.rank() == rank_0)
    {
        subproblem_per_cut_indices_ = get_subs_per_cut(gathered_subs_per_proc,
                                                       _data.control.nsubproblem);
    }
    BuildMasterProblem();
    BroadCastVariablesIndices();
    subproblems_manager_->BuildSubproblemWorkerFactory(_options.CACHE_PROBLEMS, &_world);
    init_problems_ = false;
}

std::vector<SubProblemNamesInCut> BendersMpi::get_subs_per_cut(
  const std::vector<SubProblemNamesInCut>& gathered_sub_per_proc,
  int max_aggregation)
{
    int n_cuts = SetAggregation(max_aggregation);

    std::vector<Entry> ordered(_data.control.nsubproblem);

    for (auto& proc_subs_vec: gathered_sub_per_proc)
    {
        for (auto& sub: proc_subs_vec)
        {
            auto it = _problem_to_id.find(sub.first);
            if (it == _problem_to_id.end())
            {
                continue;
            }
            ordered[it->second] = {&sub.first, sub.second};
        }
    }

    std::vector<SubProblemNamesInCut> cuts;
    cuts.reserve(n_cuts);

    SubProblemNamesInCut cut;
    cut.reserve((_data.control.nsubproblem + n_cuts - 1) / n_cuts);

    for (const auto& e: ordered)
    {
        if (!e.name)
        {
            continue;
        }

        cut.emplace_back(*e.name, e.vecPos);
        if (cut.size() == static_cast<size_t>((_data.control.nsubproblem + n_cuts - 1) / n_cuts))
        {
            cuts.emplace_back(std::move(cut));
            cut.clear();
            cut.reserve((_data.control.nsubproblem + n_cuts - 1) / n_cuts);
        }
    }

    if (!cut.empty())
    {
        cuts.emplace_back(std::move(cut));
    }
    return cuts;
}

void BendersMpi::BroadCastVariablesIndices()
{
    if (_world.rank() == rank_0)
    {
        subproblems_manager_->SetSubproblemsVariablesIndices();
    }
    BroadCast(outer_loop_manager_->GetCriterionComputation().getVarIndices(), rank_0);
}

void BendersMpi::InitializeMaster()
{
    if (_world.rank() == rank_0)
    {
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
    }
}

void BendersMpi::BuildMasterProblem()
{
    InitializeMaster();
    if (_world.rank() == rank_0)
    {
        master_manager_->AddAlphasFixingConstraints(subproblem_per_cut_indices_, _problem_to_id);
    }
}

void BendersMpi::step_1_solve_master()
{
    int success = 1;
    try
    {
        do_solve_master_create_trace_and_update_cuts();
    }
    catch (const std::exception& ex)
    {
        success = 0;
        write_exception_message(ex);
    }
    check_if_some_proc_had_a_failure(success);
    if (!exception_raised_)
    {
        cuts_manager_->BroadcastXCut();
    }
}

void BendersMpi::do_solve_master_create_trace_and_update_cuts()
{
    if (_world.rank() == rank_0)
    {
        if (SwitchToIntegerMaster(_data.control.is_in_initial_relaxation))
        {
            output_manager_->GetLogger()->LogAtSwitchToInteger();
            ActivateIntegrityConstraints();
            ResetDataPostRelaxation();
        }
        solve_master_and_create_trace();
    }
}

void BendersMpi::solve_master_and_create_trace()
{
    auto logger = output_manager_->GetLogger();

    logger->log_at_initialization(_data.control.it
                                  + output_manager_->GetNumIterationsBeforeRestart());
    logger->display_message("\tSolving master...");
    get_master_value();

    logger->log_master_solving_duration(_data.master.timer_master);

    cuts_manager_->ComputeXCut(_data,
                               Options().SEPARATION_PARAM,
                               Options().MASTER_SOLUTION_TOLERANCE);
    logger->log_iteration_candidates(output_manager_->bendersDataToLogData(_data));
}

void BendersMpi::step_2_solve_subproblems_and_build_cuts()
{
    auto logger = output_manager_->GetLogger();

    int success = 1;
    SubProblemDataMap subproblem_data_map;
    Timer walltime;
    Timer subproblems_timer_per_proc;
    logger->display_message("\tSolving subproblems...");
    try
    {
        subproblem_data_map = get_subproblem_cut_package();
        _data.cuts.subproblems_cputime = subproblems_timer_per_proc.elapsed();
    }
    catch (const std::exception& ex)
    {
        success = 0;
        write_exception_message(ex);
    }
    check_if_some_proc_had_a_failure(success);

    cuts_manager_->GatherAndBuildCuts(subproblem_data_map, walltime, exception_raised_);

    logger->LogSubproblemsSolvingCumulativeCpuTime(_data.cuts.subproblems_cumulative_cputime);
    logger->LogSubproblemsSolvingWalltime(_data.cuts.subproblems_walltime);

    if (!exception_raised_ && !outer_loop_manager_->GetCriterionComputation().IsEmpty())
    {
        ComputeSubproblemsContributionToCriteria(subproblem_data_map);

        if (Rank() == rank_0)
        {
            outer_loop_manager_->PushCriteriaForIteration(_data.criteria.criteria);
            outer_loop_manager_->UpdateMaxCriterionArea();
        }
    }
    if (Rank() == rank_0)
    {
        _data.control.cumulative_number_of_subproblem_solved += _data.control.nsubproblem;
        logger->cumulative_number_of_sub_problem_solved(
          _data.control.cumulative_number_of_subproblem_solved
          + output_manager_->GetNumOfSubProblemsSolvedBeforeResume());
    }
}

void BendersMpi::ComputeSubproblemsContributionToCriteria(
  const SubProblemDataMap& subproblem_data_map)
{
    const auto vars_size = outer_loop_manager_->GetCriterionComputation().getVarIndices().size();
    std::vector<double> criteria_per_sub_problem_per_pattern(vars_size, {});
    _data.criteria.criteria.resize(vars_size, 0.);
    std::vector<double> patterns_values_per_sub_problem_per_pattern(vars_size, {});
    _data.criteria.patterns_values.resize(vars_size, 0.);

    for (const auto& [subproblem_name, subproblem_data]: subproblem_data_map)
    {
        AddVectors<double>(criteria_per_sub_problem_per_pattern, subproblem_data.criteria);
        AddVectors<double>(patterns_values_per_sub_problem_per_pattern,
                           subproblem_data.patterns_values);
    }

    Reduce(criteria_per_sub_problem_per_pattern,
           _data.criteria.criteria,
           std::plus<double>(),
           rank_0);
    Reduce(patterns_values_per_sub_problem_per_pattern,
           _data.criteria.patterns_values,
           std::plus<double>(),
           rank_0);
}

SubProblemDataMap BendersMpi::get_subproblem_cut_package()
{
    SubProblemDataMap subproblem_data_map;
    subproblems_manager_->GetSubproblemCut(
      subproblem_data_map,
      subproblems_manager_->MakeFastBeginHook(),
      subproblems_manager_->MakeCacheBeginHook(),
      subproblems_manager_->MakePostSolveHook(outer_loop_manager_->GetCriterionComputation(),
                                              _data));
    return subproblem_data_map;
}

void BendersMpi::check_if_some_proc_had_a_failure(int success)
{
    int global_success;
    mpi::all_reduce(_world, success, global_success, mpi::bitwise_and<int>());
    if (global_success == 0)
    {
        exception_raised_ = true;
    }
}

void BendersMpi::write_exception_message(const std::exception& ex) const
{
    std::string error = "Exception raised : " + std::string(ex.what());
    output_manager_->GetLogger()->display_message(error);
}

void BendersMpi::step_4_update_best_solution(int rank)
{
    if (rank == rank_0)
    {
        compute_ub();
        update_best_ub();
        output_manager_->GetLogger()->log_at_iteration_end(
          output_manager_->bendersDataToLogData(_data));

        UpdateTrace();
        _data.control.iteration_time = -_data.control.benders_time;
        _data.control.benders_time = GetBendersTime();
        _data.control.iteration_time += _data.control.benders_time;
        _data.control.stop = ShouldBendersStop();
    }
}

void BendersMpi::free()
{
    if (_world.rank() == rank_0)
    {
        master_manager_->FreeMaster();
    }
    else
    {
        subproblems_manager_->free_subproblems();
    }
    _world.barrier();
}

void BendersMpi::Run()
{
    if (init_data_)
    {
        PreRunInitialization();
    }
    else
    {
        _data.control.stop = false;
    }
    _data.control.number_of_subproblem_solved = _data.control.nsubproblem;

    while (!_data.control.stop)
    {
        benders_plugin_->OnBendersIterationStart();

        ++_data.control.it;
        ResetSimplexIterationsBounds();

        benders_plugin_->OnBendersMasterResolutionStart();

        step_1_solve_master();

        benders_plugin_->OnBendersMasterResolutionEnd(_data.solution.x_cut, _data.control.it);

        if (!exception_raised_)
        {
            step_2_solve_subproblems_and_build_cuts();
        }

        if (!exception_raised_)
        {
            step_4_update_best_solution(_world.rank());
        }
        _data.control.stop |= exception_raised_;

        broadcast(_world, _data.control.is_in_initial_relaxation, rank_0);
        broadcast(_world, _data.control.stop, rank_0);

        if (Rank() == rank_0)
        {
            output_manager_->MathLoggerPrint();
            output_manager_->SaveCurrentBendersData(LastIterationFile(), _options.TRACE);
        }

        benders_plugin_->OnBendersIterationEnd();
    }
    if (_world.rank() == rank_0)
    {
        output_manager_->CloseCsvFile();
        output_manager_->EndWritingInOutputFile(_data.control.benders_time,
                                                _options.EXTERNAL_LOOP_OPTIONS.DO_OUTER_LOOP);
        write_basis();
    }
    _world.barrier();
}

void BendersMpi::PreRunInitialization()
{
    init_data();

    if (_world.rank() == rank_0)
    {
        HandleInitialMasterRelaxation();
    }

    _world.barrier();

    if (_world.rank() == rank_0)
    {
        ChecksResumeMode();
        if (_options.TRACE)
        {
            output_manager_->OpenCsvFile();
        }
    }
    output_manager_->MathLoggerWriteHeader();
    init_data_ = false;
}

void BendersMpi::launch()
{
    if (init_problems_)
    {
        InitializeProblems();
    }

    _world.barrier();

    benders_plugin_->OnBendersStart(subproblems_manager_->GetSubProblemMap(),
                                    output_manager_->GetLogger(),
                                    _options,
                                    solver_log_manager_,
                                    subproblems_manager_->GetFactorySolver());

    Run();

    _world.barrier();

    benders_plugin_->OnBendersEnd();

    post_run_actions();

    if (free_problems_)
    {
        free();
    }

    _world.barrier();
}
