

#include "antares-xpansion/benders/benders_mpi/BendersMPI.h"

#include <algorithm>
#include <utility>

#include "antares-xpansion/benders/benders_core/CriterionComputation.h"
#include "antares-xpansion/benders/benders_core/CustomVector.h"
#include "antares-xpansion/helpers/Timer.h"

BendersMpi::BendersMpi(BendersBaseOptions const &options, Logger logger,
                       Writer writer, mpi::environment &env,
                       mpi::communicator &world,
                       std::shared_ptr<MathLoggerDriver> mathLoggerDriver)
    : BendersBase(options, logger, std::move(writer), mathLoggerDriver),
      _world(world),
      _env(env) {}

/*!
 *  \brief Method to load each problem in a thread
 *
 *  The initialization of each problem is done sequentially
 *
 */

void BendersMpi::InitializeProblems() {
  MatchProblemToId();

  BuildMasterProblem();
  int current_problem_id = 0;
  // Dispatch subproblems to process
  for (const auto &problem : coupling_map_) {
    // In case there are more subproblems than process
    if (auto process_to_feed = current_problem_id % _world.size();
        process_to_feed ==
        _world.rank()) {  // Assign  [problemNumber % processCount] to processID

      const auto subProblemFilePath = GetSubproblemPath(problem.first);
      AddSubproblem(problem);
      AddSubproblemName(problem.first);
    }
    current_problem_id++;
  }
  BroadCastVariablesIndices();
  init_problems_ = false;
}

void BendersMpi::BroadCastVariablesIndices() {
  if (_world.rank() == rank_0) {
    SetSubproblemsVariablesIndices();
  }
  BroadCast(criterion_computation_.getVarIndices(), rank_0);
}

void BendersMpi::BuildMasterProblem() {
  if (_world.rank() == rank_0) {
    reset_master<WorkerMaster>(master_variable_map_, get_master_path(),
                                  get_solver_name(), get_log_level(),
                                  _data.nsubproblem, solver_log_manager_,
                                  IsResumeMode(), _logger);
  }
}
/*!
 *  \brief Solve, get and send solution of the Master Problem to every thread
 *
 *  \param _env : environment variable for mpi communication
 *
 *  \param _world : communicator variable for mpi communication
 */
void BendersMpi::step_1_solve_master() {
  int success = 1;
  try {
    do_solve_master_create_trace_and_update_cuts();
  } catch (std::exception const &ex) {
    success = 0;
    write_exception_message(ex);
  }
  check_if_some_proc_had_a_failure(success);
  BroadcastXCut();
}

void BendersMpi::do_solve_master_create_trace_and_update_cuts() {
  if (_world.rank() == rank_0) {
    if (SwitchToIntegerMaster(_data.is_in_initial_relaxation)) {
      _logger->LogAtSwitchToInteger();
      ActivateIntegrityConstraints();
      ResetDataPostRelaxation();
    }
    solve_master_and_create_trace();
  }
}

void BendersMpi::BroadcastXCut() {
  if (!exception_raised_) {
    Point x_cut = get_x_cut();
    mpi::broadcast(_world, x_cut, rank_0);
    set_x_cut(x_cut);
  }
}

void BendersMpi::solve_master_and_create_trace() {
  _logger->log_at_initialization(_data.it + GetNumIterationsBeforeRestart());
  _logger->display_message("\tSolving master...");
  get_master_value();
  _logger->log_master_solving_duration(get_timer_master());

  ComputeXCut();
  _logger->log_iteration_candidates(bendersDataToLogData(_data));
}

/*!
 *  \brief Get cut information from each Subproblem and add it to the Master
 * problem
 *
 * Get cut information of every Subproblem in each thread and send it to
 * thread 0 to build new Master's cuts
 *
 */
void BendersMpi::step_2_solve_subproblems_and_build_cuts() {
  int success = 1;
  SubProblemDataMap subproblem_data_map;
  Timer walltime;
  Timer subproblems_timer_per_proc;
  try {
    subproblem_data_map = get_subproblem_cut_package();
    SetSubproblemsCpuTime(subproblems_timer_per_proc.elapsed());

  } catch (std::exception const &ex) {
    success = 0;
    write_exception_message(ex);
  }
  check_if_some_proc_had_a_failure(success);
  gather_subproblems_cut_package_and_build_cuts(subproblem_data_map, walltime);
  if (Rank() == rank_0) {
    _data.cumulative_number_of_subproblem_solved += _data.nsubproblem;
    _logger->cumulative_number_of_sub_problem_solved(
        _data.cumulative_number_of_subproblem_solved +
        GetNumOfSubProblemsSolvedBeforeResume());
  }
}

void BendersMpi::gather_subproblems_cut_package_and_build_cuts(
    const SubProblemDataMap &subproblem_data_map, const Timer &walltime) {
  if (!exception_raised_) {
    GatherCuts(subproblem_data_map, walltime);
  }
}
void BendersMpi::GatherCuts(const SubProblemDataMap &subproblem_data_map,
                            const Timer &walltime) {
  std::vector<SubProblemDataMap> gathered_subproblem_map;
  mpi::gather(_world, subproblem_data_map, gathered_subproblem_map, rank_0);
  SetSubproblemsWalltime(walltime.elapsed());
  double cumulative_subproblems_timer_per_iter(0);
  Reduce(GetSubproblemsCpuTime(), cumulative_subproblems_timer_per_iter,
         std::plus<double>(), rank_0);
  SetSubproblemsCumulativeCpuTime(cumulative_subproblems_timer_per_iter);

  // only rank_0 receive non-emtpy gathered_subproblem_map
  master_build_cuts(gathered_subproblem_map);

  ComputeSubproblemsContributionToCriteria(subproblem_data_map);
  if (_world.rank() == rank_0) {
    criteria_vector_for_each_iteration_.push_back(
      _data.criteria_current_iteration_data.criteria);
    UpdateMaxCriterionArea();
  }
}

void BendersMpi::SolveSubproblem(
    SubProblemDataMap &subproblem_data_map,
    PlainData::SubProblemData &subproblem_data, const std::string &name,
    const std::shared_ptr<SubproblemWorker> &worker) {
  BendersBase::SolveSubproblem(subproblem_data_map, subproblem_data, name,
                               worker);

  std::vector<double> solution;
  worker->get_solution(solution);
  criterion_computation_.ComputeCriterion(
    SubproblemWeight(_data.nsubproblem, name), solution,
    subproblem_data.criteria,
    subproblem_data.patterns_values);
}

void BendersMpi::UpdateMaxCriterionArea() {
  auto criteria_begin =
      _data.criteria_current_iteration_data.criteria.cbegin();
  auto criteria_end =
      _data.criteria_current_iteration_data.criteria.cend();
  auto max_criterion_it = std::max_element(criteria_begin, criteria_end);
  if (max_criterion_it != criteria_end) {
    _data.criteria_current_iteration_data.max_criterion = *max_criterion_it;
    auto max_criterion_index =
        std::distance(criteria_begin, max_criterion_it);
    _data.criteria_current_iteration_data.max_criterion_area =
        criterion_computation_.getCriterionInputData()
        .Criteria()[max_criterion_index]
        .Pattern()
        .GetBody();
  }
}

void BendersMpi::ComputeSubproblemsContributionToCriteria(
  const SubProblemDataMap &subproblem_data_map) {
  const auto vars_size = criterion_computation_.getVarIndices().size();
  std::vector<double> criteria_per_sub_problem_per_pattern(
    vars_size, {});
  _data.criteria_current_iteration_data.criteria.resize(vars_size,
                                                        0.);
  std::vector<double> patterns_values_per_sub_problem_per_pattern(
    vars_size, {});
  _data.criteria_current_iteration_data.patterns_values.resize(
    vars_size, 0.);

  for (const auto &[subproblem_name, subproblem_data] : subproblem_data_map) {
    AddVectors<double>(criteria_per_sub_problem_per_pattern,
                       subproblem_data.criteria);
    AddVectors<double>(patterns_values_per_sub_problem_per_pattern,
                       subproblem_data.patterns_values);
  }

  Reduce(criteria_per_sub_problem_per_pattern,
         _data.criteria_current_iteration_data.criteria,
         std::plus<double>(), rank_0);
  Reduce(patterns_values_per_sub_problem_per_pattern,
         _data.criteria_current_iteration_data.patterns_values,
         std::plus<double>(), rank_0);
}

SubProblemDataMap BendersMpi::get_subproblem_cut_package() {
  SubProblemDataMap subproblem_data_map;
  GetSubproblemCut(subproblem_data_map);
  return subproblem_data_map;
}

void BendersMpi::master_build_cuts(
    std::vector<SubProblemDataMap> gathered_subproblem_map) {
  SetSubproblemCost(0);

  for (const auto &subproblem_data_map : gathered_subproblem_map) {
    for (auto &&[sub_problem_name, subproblem_data] : subproblem_data_map) {

      SetSubproblemCost(GetSubproblemCost() + subproblem_data.subproblem_cost);
      BoundSimplexIterations(subproblem_data.simplex_iter);
    }
  }

  _logger->display_message("\tSolving subproblems...");

  _data.ub = 0;

  for (const auto &subproblem_data_map : gathered_subproblem_map) {
    BuildCutFull(subproblem_data_map);
  }

  _logger->LogSubproblemsSolvingCumulativeCpuTime(
      GetSubproblemsCumulativeCpuTime());
  _logger->LogSubproblemsSolvingWalltime(GetSubproblemsWalltime());
}

/*!
 *  \brief Gather, store and sort all process results in a set
 *
 *  \param _env : environment variable for mpi communication
 *
 *  \param _world : communicator variable for mpi communication
 */

void BendersMpi::check_if_some_proc_had_a_failure(int success) {
  int global_success;
  mpi::all_reduce(_world, success, global_success, mpi::bitwise_and<int>());
  if (global_success == 0) {
    exception_raised_ = true;
  }
}

void BendersMpi::write_exception_message(const std::exception &ex) const {
  std::string error = "Exception raised : " + std::string(ex.what());
  _logger->display_message(error);
}

void BendersMpi::step_4_update_best_solution(int rank) {
  if (rank == rank_0) {
    compute_ub();
    update_best_ub();
    _logger->log_at_iteration_end(bendersDataToLogData(_data));

    UpdateTrace();
    _data.iteration_time = -_data.benders_time;
    _data.benders_time = GetBendersTime();
    _data.iteration_time += _data.benders_time;
    _data.stop = ShouldBendersStop();
  }
}

/*!
 *  \brief Method to free the memory used by each problem
 */
void BendersMpi::free() {
  if (_world.rank() == rank_0) {
    free_master();
  } else {
    free_subproblems();
  }
  _world.barrier();
}

/*!
 *  \brief Run Benders algorithm in parallel
 *
 *  Method to run Benders algorithm in parallel
 *
 */
void BendersMpi::Run() {
  if (init_data_) {
    PreRunInitialization();
  } else {
    // only ?
    _data.stop = false;
  }
  _data.number_of_subproblem_solved = _data.nsubproblem;
  while (!_data.stop) {
    ++_data.it;
    ResetSimplexIterationsBounds();

    /*Solve Master problem, get optimal value and cost and send it to
     * process*/
    step_1_solve_master();

    /*Gather cut from each subproblem in master thread and add them to Master
     * problem*/
    if (!exception_raised_) {
      step_2_solve_subproblems_and_build_cuts();
    }

    if (!exception_raised_) {
      step_4_update_best_solution(_world.rank());
    }
    _data.stop |= exception_raised_;

    broadcast(_world, _data.is_in_initial_relaxation, rank_0);
    broadcast(_world, _data.stop, rank_0);

    if (Rank() == rank_0) {
      mathLoggerDriver_->Print(_data);
      SaveCurrentBendersData();
    }
  }
  if (_world.rank() == rank_0) {
    CloseCsvFile();
    EndWritingInOutputFile();
    write_basis();
  }
  _world.barrier();
}

void BendersMpi::PreRunInitialization() {
  init_data();

  if (_world.rank() == rank_0) {
    if (is_initial_relaxation_requested()) {
      _logger->LogAtInitialRelaxation();
      DeactivateIntegrityConstraints();
      SetDataPreRelaxation();
    }
  }

  _world.barrier();

  if (_world.rank() == rank_0) {
    ChecksResumeMode();
    if (is_trace()) {
      OpenCsvFile();
    }

  }
  mathLoggerDriver_->write_header();
  init_data_ = false;
}

void BendersMpi::launch() {
  if (init_problems_) {
    InitializeProblems();
  }
  _world.barrier();

  Run();
  _world.barrier();

  post_run_actions();

  if (free_problems_) {
    free();
  }
  _world.barrier();
}
