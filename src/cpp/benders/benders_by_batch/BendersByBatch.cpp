#include "antares-xpansion/benders/benders_by_batch/BendersByBatch.h"

#include <mutex>
#include <numeric>

#include "antares-xpansion/benders/benders_by_batch/BatchCollection.h"
#include "antares-xpansion/benders/benders_by_batch/RandomBatchShuffler.h"

BendersByBatch::BendersByBatch(const BendersBaseOptions& options,
                               std::shared_ptr<ILogger> logger,
                               std::shared_ptr<Output::OutputWriter> writer,
                               mpi::communicator& world,
                               std::shared_ptr<MathLoggerDriver> mathLoggerDriver):
    BendersMpi(options, logger, std::move(writer), world, std::move(mathLoggerDriver)),
    batch_cuts_manager_(std::make_shared<BendersCutsManagerByBatch>(world,
                                                                    rank_0,
                                                                    _data,
                                                                    _problem_to_id,
                                                                    relevantIterationData_,
                                                                    _master)),
    batch_subproblems_manager_(
      std::make_shared<BendersSubProblemsManagerByBatch>(_data,
                                                         _options,
                                                         benders_plugin_,
                                                         output_manager_->GetLogger(),
                                                         solver_log_manager_,
                                                         output_manager_->GetWriter(),
                                                         shouldParallelize()))
{
    batch_subproblems_manager_->SetOnVariablesIndicesSet(
      [this](const std::vector<std::string>& col_names)
      { outer_loop_manager_->GetCriterionComputation().SearchVariables(col_names); });
}

void BendersByBatch::free()
{
    if (_world.rank() == rank_0)
    {
        master_manager_->FreeMaster();
    }
    else
    {
        batch_subproblems_manager_->free_subproblems();
    }
    _world.barrier();
}

void BendersByBatch::launch()
{
    if (init_problems_)
    {
        InitializeProblems();
    }

    _world.barrier();

    benders_plugin_->OnBendersStart(batch_subproblems_manager_->GetSubProblemMap(),
                                    output_manager_->GetLogger(),
                                    _options,
                                    solver_log_manager_,
                                    batch_subproblems_manager_->GetFactorySolver());

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

void BendersByBatch::BroadCastVariablesIndices()
{
    if (_world.rank() == rank_0)
    {
        batch_subproblems_manager_->SetSubproblemsVariablesIndices();
    }
    BroadCast(outer_loop_manager_->GetCriterionComputation().getVarIndices(), rank_0);
}

void BendersByBatch::InitializeProblems()
{
    MatchProblemToId();
    BuildBatches();
    BuildMasterProblem();
    BroadCastVariablesIndices();
    batch_subproblems_manager_->BuildSubproblemWorkerFactory(_options.CACHE_PROBLEMS, &_world);
    init_problems_ = false;
}

void BendersByBatch::BuildMasterProblem()
{
    InitializeMaster();
    for (auto& batch: batch_collection_full_for_cuts_.BatchCollections())
    {
        master_manager_->AddAlphasFixingConstraints(batch.name_to_cut, _problem_to_id);
    }
}

void BendersByBatch::BuildBatches()
{
    const auto& coupling_map_size = coupling_map_.size();

    if (Rank() == rank_0)
    {
        std::vector<std::string> problem_names;
        for (const auto& problem_name: coupling_map_ | std::views::keys)
        {
            problem_names.emplace_back(problem_name);
        }
        auto batch_size = Options().BATCH_SIZE == 0 ? coupling_map_size : Options().BATCH_SIZE;
        batch_collection_.SetLogger(output_manager_->GetLogger());
        batch_collection_.SetBatchSize(batch_size);
        batch_collection_.SetSubProblemNames(problem_names);
        batch_collection_.BuildBatches(WorldSize());
        batch_collection_full_for_cuts_ = batch_collection_;
        get_subs_per_cut_per_batch();
    }
    BroadCast(batch_collection_, rank_0);

    auto problem_count = 0;

    for (auto& batch: batch_collection_.BatchCollections())
    {
        switch (_options.CACHE_PROBLEMS)
        {
        case 1:
        case 2:
        {
            for (auto it = batch.sub_problem_names.begin(); it != batch.sub_problem_names.end();)
            {
                auto process_to_feed = problem_count % WorldSize();
                if (process_to_feed != Rank())
                {
                    it = batch.sub_problem_names.erase(it);
                }
                else
                {
                    batch_subproblems_manager_->AddSubproblemName(*it);
                    ++it;
                }
                ++problem_count;
            }
            batch.sub_problem_names.shrink_to_fit();
            break;
        }
        case 0:
        default:
        {
            for (auto it = batch.sub_problem_names.begin(); it != batch.sub_problem_names.end();)
            {
                auto process_to_feed = problem_count % WorldSize();
                if (process_to_feed != Rank())
                {
                    it = batch.sub_problem_names.erase(it);
                }
                else
                {
                    batch_subproblems_manager_->AddSubproblem({*it, coupling_map_[*it]});
                    batch_subproblems_manager_->AddSubproblemName(*it);
                    ++it;
                }
                ++problem_count;
            }
            batch.sub_problem_names.shrink_to_fit();
            break;
        }
        }
    }
    batch_subproblems_manager_->SetCouplingMap(coupling_map_);

    BroadCastVariablesIndices();
    init_problems_ = false;
}

void BendersByBatch::get_subs_per_cut_per_batch()
{
    for (auto& batch: batch_collection_full_for_cuts_.BatchCollections())
    {
        int n_cuts = SetAggregation(batch.sub_problem_names.size());
        batch.AssociateSubProblemsToCut(n_cuts);
    }
}

void BendersByBatch::BroadcastSingleSubpbCostsUnderApprox()
{
    DblVector single_subpb_costs_under_approx(_data.control.nsubproblem);
    if (Rank() == rank_0)
    {
        single_subpb_costs_under_approx = GetAlpha_i();
    }

    BroadCast(single_subpb_costs_under_approx.data(), _data.control.nsubproblem, rank_0);
    SetAlpha_i(single_subpb_costs_under_approx);
}

void BendersByBatch::Run()
{
    auto logger = output_manager_->GetLogger();

    if (init_data_)
    {
        PreRunInitialization();
    }
    else
    {
        _data.control.stop = false;
    }

    MasterLoop();

    if (Rank() == rank_0)
    {
        compute_ub();
        update_best_ub();
        logger->log_at_iteration_end(output_manager_->bendersDataToLogData(_data));
        UpdateTrace();
        output_manager_->SaveCurrentBendersData(LastIterationFile(), _options.TRACE);
        output_manager_->CloseCsvFile();
        output_manager_->EndWritingInOutputFile(_data.control.benders_time,
                                                _options.EXTERNAL_LOOP_OPTIONS.DO_OUTER_LOOP);
        write_basis();
    }
}

void BendersByBatch::MasterLoop()
{
    auto logger = output_manager_->GetLogger();

    number_of_batch_ = batch_collection_.NumberOfBatch();
    random_batch_permutation_.resize(number_of_batch_);
    batch_counter_ = 0;
    current_batch_id_ = 0;
    _data.control.number_of_subproblem_solved = 0;
    cumulative_subproblems_timer_per_iter_ = 0;
    first_unsolved_batch_ = 0;
    while (!_data.control.stop)
    {
        benders_plugin_->OnBendersIterationStart();

        if (Rank() == rank_0)
        {
            if (SwitchToIntegerMaster(_data.control.is_in_initial_relaxation))
            {
                logger->LogAtSwitchToInteger();
                ActivateIntegrityConstraints();
                ResetDataPostRelaxation();
            }
        }

        _data.cuts.ub = 0;
        _data.cuts.subproblem_cost = 0;
        remaining_epsilon_ = Gap();

        benders_plugin_->OnBendersMasterResolutionStart();
        if (Rank() == rank_0)
        {
            logger->PrintIterationSeparatorBegin();

            logger->display_message("\tSolving master...");
            get_master_value();
            logger->log_master_solving_duration(_data.master.timer_master);

            random_batch_permutation_ = RandomBatchShuffler(number_of_batch_)
                                          .GetCyclicBatchOrder(current_batch_id_);
        }
        BroadcastXOut();
        BroadcastSingleSubpbCostsUnderApprox();
        BroadCast(random_batch_permutation_.data(), random_batch_permutation_.size(), rank_0);
        SeparationLoop();
        if (Rank() == rank_0)
        {
            _data.control.iteration_time = -_data.control.benders_time;
            _data.control.benders_time = GetBendersTime();
            _data.control.iteration_time += _data.control.benders_time;
            _data.control.stop = ShouldBendersStop();
        }
        BroadCast(_data.control.stop, rank_0);
        BroadCast(batch_counter_, rank_0);
        _data.cuts.subproblems_cumulative_cputime = cumulative_subproblems_timer_per_iter_;
        logger->cumulative_number_of_sub_problem_solved(
          _data.control.cumulative_number_of_subproblem_solved
          + output_manager_->GetNumOfSubProblemsSolvedBeforeResume());
        logger->LogSubproblemsSolvingCumulativeCpuTime(_data.cuts.subproblems_cumulative_cputime);
        logger->LogSubproblemsSolvingWalltime(_data.cuts.subproblems_walltime);
        logger->PrintIterationSeparatorEnd();
        output_manager_->MathLoggerPrint();

        benders_plugin_->OnBendersIterationEnd();
    }
}

void BendersByBatch::SeparationLoop()
{
    auto logger = output_manager_->GetLogger();

    misprice_ = true;
    first_unsolved_batch_ = 0;
    batch_counter_ = 0;
    while (misprice_ && batch_counter_ < number_of_batch_)
    {
        _data.control.it++;
        ResetSimplexIterationsBounds();

        logger->log_at_initialization(_data.control.it
                                      + output_manager_->GetNumIterationsBeforeRestart());
        if (Rank() == rank_0)
        {
            ComputeXCut();
        }
        batch_cuts_manager_->BroadcastXCut();

        benders_plugin_->OnBendersMasterResolutionEnd(_data.solution.x_cut, _data.control.it);
        logger->log_iteration_candidates(output_manager_->bendersDataToLogData(_data));
        UpdateRemainingEpsilon();
        _data.control.number_of_subproblem_solved = 0;
        SolveBatches();

        if (Rank() == rank_0)
        {
            outer_loop_manager_->PushCriteriaForIteration(_data.criteria.criteria);
            UpdateTrace();
            output_manager_->SaveCurrentBendersData(LastIterationFile(), _options.TRACE);
        }
        ClearCurrentIterationCutTrace();
    }
}

void BendersByBatch::ComputeXCut()
{
    if (_data.control.it != 1)
    {
        _data.solution.x_in = _data.solution.x_cut;
        _data.solution.master_only_vars_in = _data.solution.master_only_vars_cut;
    }
    batch_cuts_manager_->ComputeXCut(_data,
                                     Options().SEPARATION_PARAM,
                                     Options().MASTER_SOLUTION_TOLERANCE);
}

void BendersByBatch::UpdateRemainingEpsilon()
{
    if (Rank() == rank_0)
    {
        auto obj = master_manager_->GetObjectiveFunctionCoeffs();
        const auto& name_to_id = master_manager_->GetNameToId();
        remaining_epsilon_ = Gap();
        for (const auto& [candidate_name, x_cut_candidate_value]: _data.solution.x_cut)
        {
            int col_id = name_to_id.at(candidate_name);
            remaining_epsilon_ -= obj[col_id]
                                  * (x_cut_candidate_value - _data.solution.x_out[candidate_name]);
        }
    }
}

void BendersByBatch::SolveBatches()
{
    batch_counter_ = 0;
    cumulative_subproblems_timer_per_iter_ = 0;
    int problem_solved_by_rank = 0;
    while (batch_counter_ < number_of_batch_)
    {
        first_unsolved_batch_ = first_unsolved_batch_ % number_of_batch_;
        current_batch_id_ = random_batch_permutation_[first_unsolved_batch_];
        first_unsolved_batch_++;
        const auto& batch = batch_collection_.GetBatchFromId(current_batch_id_);
        const auto& batch_sub_problems = batch.sub_problem_names;
        double batch_contribution_in_gap = 0;
        std::vector<double> external_loop_criterion_current_batch = {};
        int problem_solved = 0;
        BuildCut(batch_sub_problems,
                 &batch_contribution_in_gap,
                 external_loop_criterion_current_batch,
                 problem_solved);
        problem_solved_by_rank += problem_solved;
        Reduce(_data.cuts.subproblems_cputime,
               cumulative_subproblems_timer_per_iter_,
               std::plus<double>(),
               rank_0);
        current_batch_id_++;

        if (Rank() == rank_0)
        {
            remaining_epsilon_ -= batch_contribution_in_gap;
        }

        BroadCast(remaining_epsilon_, rank_0);
        if (remaining_epsilon_ > 0)
        {
            batch_counter_++;
        }
        else
        {
            break;
        }
    }

    int global_total_solved = 0;
    Reduce(problem_solved_by_rank, global_total_solved, std::plus<int>(), rank_0);
    if (Rank() == rank_0)
    {
        _data.control.number_of_subproblem_solved = global_total_solved;
        _data.control.cumulative_number_of_subproblem_solved += global_total_solved;
    }
}

void BendersByBatch::BuildCut(const std::vector<std::string>& batch_sub_problems,
                              double* batch_contribution_in_gap,
                              std::vector<double>& external_loop_criterion_current_batch,
                              int& local_solved)
{
    SubProblemDataMap subproblem_data_map;
    Timer subproblems_timer_per_proc;
    batch_subproblems_manager_->GetSubproblemCut(
      subproblem_data_map,
      batch_subproblems_manager_->MakeFastBeginHook(batch_sub_problems),
      batch_subproblems_manager_->MakeCacheBeginHook(batch_sub_problems),
      batch_subproblems_manager_->MakePostSolveHook(
        [this](const std::string& name, PlainData::SubProblemData& data)
        { calculate_subproblem_contribution(name, data); }));
    local_solved = subproblem_data_map.size();

    _data.cuts.subproblems_cputime = subproblems_timer_per_proc.elapsed();

    bool global_misprice = misprice_;
    AllReduce(misprice_, global_misprice, std::logical_and<bool>());
    misprice_ = global_misprice;

    auto& batch_cuts_list = batch_collection_full_for_cuts_.BatchCollections();
    batch_cuts_manager_->GatherAndBuildCuts(subproblem_data_map,
                                            subproblems_timer_per_proc,
                                            batch_cuts_list[current_batch_id_].name_to_cut,
                                            *batch_contribution_in_gap);
}

void BendersByBatch::calculate_subproblem_contribution(const std::string& name,
                                                       PlainData::SubProblemData& subproblem_data)
{
    auto subpb_cost_under_approx = GetAlpha_i()[ProblemToId(name)];
    subproblem_data.contribution_in_gap = subproblem_data.subproblem_cost - subpb_cost_under_approx;
    double cut_value_at_x_cut = subproblem_data.subproblem_cost;
    for (const auto& [candidate_name, x_cut_candidate_value]: _data.solution.x_cut)
    {
        auto subgradient_at_name = subproblem_data.var_name_and_subgradient[candidate_name];
        cut_value_at_x_cut += subgradient_at_name
                              * (_data.solution.x_out[candidate_name] - x_cut_candidate_value);
    }

    if (subpb_cost_under_approx < cut_value_at_x_cut)
    {
        misprice_ = false;
    }
}

void BendersByBatch::BroadcastXOut()
{
    Point x_out = _data.solution.x_out;
    BroadCast(x_out, rank_0);
    _data.solution.x_out = x_out;
}

double BendersByBatch::Gap() const
{
    if (_data.control.is_in_initial_relaxation)
    {
        return RelaxedGap() * _data.master.lb;
    }
    else
    {
        return (std::max)(AbsoluteGap(), RelativeGap() * _data.master.lb);
    }
}

void BendersByBatch::UpdateStoppingCriterion()
{
    if (_data.control.benders_time > Options().TIME_LIMIT)
    {
        _data.control.stopping_criterion = StoppingCriterion::timelimit;
    }
    else if ((Options().MAX_ITERATIONS != -1) && (_data.control.it >= Options().MAX_ITERATIONS))
    {
        _data.control.stopping_criterion = StoppingCriterion::max_iteration;
    }
    else if (batch_counter_ >= number_of_batch_)
    {
        if (Gap() == AbsoluteGap())
        {
            _data.control.stopping_criterion = StoppingCriterion::absolute_gap;
        }
        else
        {
            _data.control.stopping_criterion = StoppingCriterion::relative_gap;
        }
    }
}

bool BendersByBatch::ShouldRelaxationStop() const
{
    return (_data.control.stopping_criterion != StoppingCriterion::empty);
}
