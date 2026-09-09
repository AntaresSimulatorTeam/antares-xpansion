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
    batch_cuts_manager_(world, rank_0, _data, _problem_to_id, relevantIterationData_, _master),
    batch_subproblems_manager_(_data,
                               _options,
                               benders_plugin_,
                               _logger,
                               solver_log_manager_,
                               _writer,
                               shouldParallelize())
{
}

void BendersByBatch::free()
{
    if (_world.rank() == rank_0)
    {
        free_master();
    }
    else
    {
        batch_subproblems_manager_.free_subproblems();
    }
    _world.barrier();
}

// TODO: unify launch() in BendersBase with virtual Synchronize() and
// GetActiveSubProblemMap()/GetActiveFactorySolver() accessors so all 3
// benders variants share the same launch logic.
void BendersByBatch::launch()
{
    if (init_problems_)
    {
        InitializeProblems();
    }

    _world.barrier();

    benders_plugin_->OnBendersStart(batch_subproblems_manager_.GetSubProblemMap(),
                                    _logger,
                                    _options,
                                    solver_log_manager_,
                                    batch_subproblems_manager_.GetFactorySolver());

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
        batch_subproblems_manager_.SetSubproblemsVariablesIndices();
    }
    BroadCast(criterion_computation_.getVarIndices(), rank_0);
}

void BendersByBatch::InitializeProblems()
{
    MatchProblemToId();
    BuildBatches();
    BuildMasterProblem();
    BroadCastVariablesIndices();
    batch_subproblems_manager_.BuildSubproblemWorkerFactory(_options.CACHE_PROBLEMS, &_world);
    init_problems_ = false;
}

void BendersByBatch::BuildMasterProblem()
{
    InitializeMaster();
    for (auto& batch: batch_collection_full_for_cuts_.BatchCollections())
    {
        _master->addAlphasFixingConstraints(batch.name_to_cut, _problem_to_id);
    }
}

void BendersByBatch::BuildBatches()
{
    const auto& coupling_map_size = coupling_map_.size();

    // Only rank 0 builds the batch collection, then it is broadcasted to all procs

    if (Rank() == rank_0)
    {
        std::vector<std::string> problem_names;
        for (const auto& problem_name: coupling_map_ | std::views::keys)
        {
            problem_names.emplace_back(problem_name);
        }
        auto batch_size = Options().BATCH_SIZE == 0 ? coupling_map_size : Options().BATCH_SIZE;
        batch_collection_.SetLogger(_logger);
        batch_collection_.SetBatchSize(batch_size);
        batch_collection_.SetSubProblemNames(problem_names);
        batch_collection_.BuildBatches(WorldSize());
        batch_collection_full_for_cuts_ = batch_collection_;
        get_subs_per_cut_per_batch();
    }
    BroadCast(batch_collection_, rank_0);

    // Dispatch subproblems to process: only add those assigned to this rank
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
                    batch_subproblems_manager_.AddSubproblemName(*it);
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
                    batch_subproblems_manager_.AddSubproblem({*it, coupling_map_[*it]});
                    batch_subproblems_manager_.AddSubproblemName(*it);
                    ++it;
                }
                ++problem_count;
            }
            batch.sub_problem_names.shrink_to_fit();
            break;
        }
        }
    }
    batch_subproblems_manager_.SetCouplingMap(coupling_map_);

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
    DblVector single_subpb_costs_under_approx(_data.nsubproblem);
    if (Rank() == rank_0)
    {
        single_subpb_costs_under_approx = GetAlpha_i();
    }

    BroadCast(single_subpb_costs_under_approx.data(), _data.nsubproblem, rank_0);
    SetAlpha_i(single_subpb_costs_under_approx);
}

void BendersByBatch::Run()
{
    if (init_data_)
    {
        PreRunInitialization();
    }
    else
    {
        _data.stop = false;
    }

    MasterLoop();

    if (Rank() == rank_0)
    {
        compute_ub();
        update_best_ub();
        _logger->log_at_iteration_end(bendersDataToLogData(_data));
        UpdateTrace();
        SaveCurrentBendersData();
        CloseCsvFile();
        EndWritingInOutputFile();
        write_basis();
    }
}

void BendersByBatch::MasterLoop()
{
    number_of_batch_ = batch_collection_.NumberOfBatch();
    random_batch_permutation_.resize(number_of_batch_);
    batch_counter_ = 0;
    current_batch_id_ = 0;
    _data.number_of_subproblem_solved = 0;
    cumulative_subproblems_timer_per_iter_ = 0;
    first_unsolved_batch_ = 0;
    while (!_data.stop)
    {
        benders_plugin_->OnBendersIterationStart();

        if (Rank() == rank_0)
        {
            if (SwitchToIntegerMaster(_data.is_in_initial_relaxation))
            {
                _logger->LogAtSwitchToInteger();
                ActivateIntegrityConstraints();
                ResetDataPostRelaxation();
            }
        }

        _data.ub = 0;
        SetSubproblemCost(0);
        remaining_epsilon_ = Gap();

        benders_plugin_->OnBendersMasterResolutionStart();
        if (Rank() == rank_0)
        {
            _logger->PrintIterationSeparatorBegin();

            _logger->display_message("\tSolving master...");
            get_master_value();
            _logger->log_master_solving_duration(_data.timer_master);

            random_batch_permutation_ = RandomBatchShuffler(number_of_batch_)
                                          .GetCyclicBatchOrder(current_batch_id_);
        }
        BroadcastXOut();
        BroadcastSingleSubpbCostsUnderApprox();
        BroadCast(random_batch_permutation_.data(), random_batch_permutation_.size(), rank_0);
        SeparationLoop();
        if (Rank() == rank_0)
        {
            _data.iteration_time = -_data.benders_time;
            _data.benders_time = GetBendersTime();
            _data.iteration_time += _data.benders_time;
            _data.stop = ShouldBendersStop();
        }
        BroadCast(_data.stop, rank_0);
        BroadCast(batch_counter_, rank_0);
        _data.subproblems_cumulative_cputime = cumulative_subproblems_timer_per_iter_;
        _logger->cumulative_number_of_sub_problem_solved(
          _data.cumulative_number_of_subproblem_solved + GetNumOfSubProblemsSolvedBeforeResume());
        _logger->LogSubproblemsSolvingCumulativeCpuTime(_data.subproblems_cumulative_cputime);
        _logger->LogSubproblemsSolvingWalltime(_data.subproblems_walltime);
        _logger->PrintIterationSeparatorEnd();
        mathLoggerDriver_->Print(_data);

        benders_plugin_->OnBendersIterationEnd();
    }
}

void BendersByBatch::SeparationLoop()
{
    misprice_ = true;
    first_unsolved_batch_ = 0;
    batch_counter_ = 0;
    while (misprice_ && batch_counter_ < number_of_batch_)
    {
        _data.it++;
        ResetSimplexIterationsBounds();

        _logger->log_at_initialization(_data.it + GetNumIterationsBeforeRestart());
        if (Rank() == rank_0)
        {
            ComputeXCut();
        }
        batch_cuts_manager_.BroadcastXCut();

        benders_plugin_->OnBendersMasterResolutionEnd(_data.x_cut, _data.it);
        _logger->log_iteration_candidates(bendersDataToLogData(_data));
        UpdateRemainingEpsilon();
        _data.number_of_subproblem_solved = 0;
        SolveBatches();

        if (Rank() == rank_0)
        {
            criteria_vector_for_each_iteration_.push_back(
              _data.criteria_current_iteration_data.criteria);
            // TODO
            //  UpdateOuterLoopMaxCriterionArea();
            UpdateTrace();
            SaveCurrentBendersData();
        }
        ClearCurrentIterationCutTrace();
    }
}

void BendersByBatch::ComputeXCut()
{
    // In batch mode, x_in must be updated before the separation formula:
    // - it==1: handled by the base ComputeXCut (sets x_in = x_out)
    // - it>1: previous x_cut becomes the new x_in for the next separation
    if (_data.it != 1)
    {
        _data.x_in = _data.x_cut;
        _data.master_only_vars_in = _data.master_only_vars_cut;
    }
    batch_cuts_manager_.ComputeXCut(_data,
                                    Options().SEPARATION_PARAM,
                                    Options().MASTER_SOLUTION_TOLERANCE);
}

void BendersByBatch::UpdateRemainingEpsilon()
{
    if (Rank() == rank_0)
    {
        auto master_ptr = get_master();
        int ncols = master_ptr->_solver->get_ncols();
        std::vector<double> obj(ncols);
        master_ptr->_solver->get_obj(obj.data(), 0, ncols - 1);
        remaining_epsilon_ = Gap();
        for (const auto& [candidate_name, x_cut_candidate_value]: _data.x_cut)
        {
            int col_id = master_ptr->_name_to_id[candidate_name];
            remaining_epsilon_ -= obj[col_id]
                                  * (x_cut_candidate_value - _data.x_out[candidate_name]);
        }
    }
}

void BendersByBatch::SolveBatches()
{
    batch_counter_ = 0;
    cumulative_subproblems_timer_per_iter_ = 0;
    // total number of subproblems solved locally on this rank during this separation iteration
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
        // Count how many subproblems this rank actually solved for the batch
        int problem_solved = 0;
        BuildCut(batch_sub_problems,
                 &batch_contribution_in_gap,
                 external_loop_criterion_current_batch,
                 problem_solved);
        // accumulate locally for the whole separation iteration
        problem_solved_by_rank += problem_solved;
        Reduce(_data.subproblems_cputime,
               cumulative_subproblems_timer_per_iter_,
               std::plus<double>(),
               rank_0);
        current_batch_id_++;

        if (Rank() == rank_0)
        {
            remaining_epsilon_ -= batch_contribution_in_gap;
            // TODO: external loop contribution aggregation
            // AddVectors<double>(_data.outer_loop_current_iteration_data.outer_loop_criterion,
            //                    external_loop_criterion_current_batch);
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

    // After processing all batches of this separation iteration, gather per-rank totals.
    int global_total_solved = 0;
    Reduce(problem_solved_by_rank, global_total_solved, std::plus<int>(), rank_0);
    if (Rank() == rank_0)
    {
        // per-iteration number of subproblems solved
        _data.number_of_subproblem_solved = global_total_solved;
        // accumulate globally across iterations
        _data.cumulative_number_of_subproblem_solved += global_total_solved;
    }
}

/*!
 * \brief Build subproblem cut
 *
 * Method to build subproblem cuts
 * and add them to the Master problem
 */
void BendersByBatch::BuildCut(const std::vector<std::string>& batch_sub_problems,
                              double* batch_contribution_in_gap,
                              std::vector<double>& external_loop_criterion_current_batch,
                              int& local_solved)
{
    SubProblemDataMap subproblem_data_map;
    Timer subproblems_timer_per_proc;
    batch_subproblems_manager_.GetSubproblemCut(
      subproblem_data_map,
      batch_subproblems_manager_.MakeFastBeginHook(batch_sub_problems),
      batch_subproblems_manager_.MakeCacheBeginHook(batch_sub_problems),
      batch_subproblems_manager_.MakePostSolveHook(
        [this](const std::string& name, PlainData::SubProblemData& data)
        { calculate_subproblem_contribution(name, data); }));
    local_solved = subproblem_data_map.size();

    _data.subproblems_cputime = subproblems_timer_per_proc.elapsed();

    bool global_misprice = misprice_;
    AllReduce(misprice_, global_misprice, std::logical_and<bool>());
    misprice_ = global_misprice;

    auto& batch_cuts_list = batch_collection_full_for_cuts_.BatchCollections();
    batch_cuts_manager_.GatherAndBuildCuts(subproblem_data_map,
                                           subproblems_timer_per_proc,
                                           batch_cuts_list[current_batch_id_].name_to_cut,
                                           *batch_contribution_in_gap);
}

void BendersByBatch::calculate_subproblem_contribution(const std::string& name,
                                                       PlainData::SubProblemData& subproblem_data)
{
    auto subpb_cost_under_approx = GetAlpha_i()[ProblemToId(name)];
    // Tbb includes min max define of windows std::numeric_limits<int>::max();
    subproblem_data.contribution_in_gap = subproblem_data.subproblem_cost - subpb_cost_under_approx;
    double cut_value_at_x_cut = subproblem_data.subproblem_cost;
    for (const auto& [candidate_name, x_cut_candidate_value]: _data.x_cut)
    {
        auto subgradient_at_name = subproblem_data.var_name_and_subgradient[candidate_name];
        cut_value_at_x_cut += subgradient_at_name
                              * (_data.x_out[candidate_name] - x_cut_candidate_value);
    }

    if (subpb_cost_under_approx < cut_value_at_x_cut)
    {
        misprice_ = false;
    }
}

void BendersByBatch::BroadcastXOut()
{
    Point x_out = get_x_out();
    BroadCast(x_out, rank_0);
    set_x_out(x_out);
}

double BendersByBatch::Gap() const
{
    if (_data.is_in_initial_relaxation)
    {
        return RelaxedGap() * _data.lb;
    }
    else
    {
        // Tbb 2020 includes Windows min max defines
        return (std::max)(AbsoluteGap(), RelativeGap() * _data.lb);
    }
}

/*!
 *  \brief Update stopping criterion
 *
 *  Method updating the stopping criterion and
 *  reinitializing some datas
 */
void BendersByBatch::UpdateStoppingCriterion()
{
    if (_data.benders_time > Options().TIME_LIMIT)
    {
        _data.stopping_criterion = StoppingCriterion::timelimit;
    }
    else if ((Options().MAX_ITERATIONS != -1) && (_data.it >= Options().MAX_ITERATIONS))
    {
        _data.stopping_criterion = StoppingCriterion::max_iteration;
    }
    else if (batch_counter_ >= number_of_batch_)
    {
        if (Gap() == AbsoluteGap())
        {
            _data.stopping_criterion = StoppingCriterion::absolute_gap;
        }
        else
        {
            _data.stopping_criterion = StoppingCriterion::relative_gap;
        }
    }
}

/*!
 *  \brief Check if initial relaxation should stop
 */
bool BendersByBatch::ShouldRelaxationStop() const
{
    return (_data.stopping_criterion != StoppingCriterion::empty);
}
