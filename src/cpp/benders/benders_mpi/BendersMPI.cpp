
#include "BendersMPI.h"

#include <algorithm>
#include <utility>

#include "Timer.h"
#include "glog/logging.h"

BendersMpi::BendersMpi(BendersBaseOptions const &options, Logger &logger,
                       Writer writer, mpi::environment &env,
                       mpi::communicator &world)
    : BendersBase(options, logger, std::move(writer)),
      _env(env),
      _world(world) {}

/*!
 *  \brief Method to load each problem in a thread
 *
 *  The initialization of each problem is done sequentially
 *
 */

void BendersMpi::initialize_problems() {
  match_problem_to_id();

  int current_problem_id = 0;
  auto subproblemProcessCount = _world.size() - 1;

  if (_world.rank() == rank_0) {
    reset_master(new WorkerMaster(
        master_variable_map, get_master_path(), get_solver_name(),
        get_log_level(), _data.nsubproblem, log_name(), IsResumeMode()));
    LOG(INFO) << "subproblem number is " << _data.nsubproblem << std::endl;
  } else {
    // Dispatch subproblems to process
    for (const auto &problem : coupling_map) {
      // In case there are more subproblems than process
      if (auto process_to_feed =
              current_problem_id % subproblemProcessCount + 1;
          process_to_feed ==
          _world
              .rank()) {  // Assign  [problemNumber % processCount] to processID
        addSubproblem(problem);
        AddSubproblemName(problem.first);
      }
      current_problem_id++;
    }
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
  broadcast_the_master_problem();
}

void BendersMpi::do_solve_master_create_trace_and_update_cuts() {
  if (_world.rank() == rank_0) {
    if (switch_to_integer_master(_data.is_in_initial_relaxation)) {
      _logger->log_at_switch_to_integer();
      activate_integrity_constraints();
      reset_data_post_relaxation();
    }
    solve_master_and_create_trace();
  }
}

void BendersMpi::broadcast_the_master_problem() {
  if (!_exceptionRaised) {
    Point x_cut = get_x_cut();
    mpi::broadcast(_world, x_cut, rank_0);
    set_x_cut(x_cut);
    _world.barrier();
  }
}

void BendersMpi::solve_master_and_create_trace() {
  _logger->log_at_initialization(_data.it + GetNumIterationsBeforeRestart());
  _logger->display_message("\tSolving master...");
  get_master_value();
  _logger->log_master_solving_duration(get_timer_master());

  compute_x_cut();
  _logger->log_iteration_candidates(bendersDataToLogData(_data));

  push_in_trace(std::make_shared<WorkerMasterData>());
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
  SubproblemCutPackage subproblem_cut_package;
  Timer process_timer;
  try {
    if (_world.rank() != rank_0) {
      subproblem_cut_package = get_subproblem_cut_package();
    }
  } catch (std::exception const &ex) {
    success = 0;
    write_exception_message(ex);
  }
  check_if_some_proc_had_a_failure(success);
  gather_subproblems_cut_package_and_build_cuts(subproblem_cut_package,
                                                process_timer);
}

void BendersMpi::gather_subproblems_cut_package_and_build_cuts(
    const SubproblemCutPackage &subproblem_cut_package,
    const Timer &process_timer) {
  if (!_exceptionRaised) {
    if (_world.rank() != rank_0) {
      mpi::gather(_world, subproblem_cut_package, rank_0);
    } else {
      AllCutPackage all_package;
      mpi::gather(_world, subproblem_cut_package, all_package, rank_0);
      SetSubproblemTimers(process_timer.elapsed());
      master_build_cuts(all_package);
    }
  }
}

SubproblemCutPackage BendersMpi::get_subproblem_cut_package() {
  SubproblemCutPackage subproblem_cut_package;
  getSubproblemCut(subproblem_cut_package);
  return subproblem_cut_package;
}

void BendersMpi::master_build_cuts(AllCutPackage all_package) {
  SetSubproblemCost(0);
  for (auto const &pack : all_package) {
    for (auto &&[_, subproblem_cut_package] : pack) {
      SetSubproblemCost(GetSubproblemCost() +
                        subproblem_cut_package.first.second[SUBPROBLEM_COST]);
    }
  }
  all_package.erase(all_package.begin());

  _logger->display_message("\tSolving subproblems...");

  build_cut_full(all_package);
  _logger->log_subproblems_solving_duration(GetSubproblemTimers());
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
    _exceptionRaised = true;
  }
}

void BendersMpi::write_exception_message(const std::exception &ex) const {
  std::string error = "Exception raised : " + std::string(ex.what());
  LOG(WARNING) << error << std::endl;
  _logger->display_message(error);
}

void BendersMpi::step_4_update_best_solution(int rank,
                                             const Timer &timer_master) {
  if (rank == rank_0) {
    compute_ub();
    update_best_ub();
    _logger->log_at_iteration_end(bendersDataToLogData(_data));

    update_trace();

    _data.elapsed_time = GetBendersTime();
    set_timer_master(timer_master.elapsed());
    _data.stop = stopping_criterion();
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
void BendersMpi::run() {
  init_data();

  if (_world.rank() == rank_0) {
    set_cut_storage();

    if (is_initial_relaxation_requested()) {
      _logger->log_at_initial_relaxation();
      deactivate_integrity_constraints();
      set_data_pre_relaxation();
    }
  }

  _world.barrier();

  if (_world.rank() == rank_0) {
    ChecksResumeMode();
    if (is_trace()) {
      OpenCsvFile();
    }
  }
  while (!_data.stop) {
    Timer timer_master;
    ++_data.it;

    /*Solve Master problem, get optimal value and cost and send it to process*/
    step_1_solve_master();

    /*Gather cut from each subproblem in master thread and add them to Master
     * problem*/
    if (!_exceptionRaised) {
      step_2_solve_subproblems_and_build_cuts();
    }

    if (!_exceptionRaised) {
      step_4_update_best_solution(_world.rank(), timer_master);
    }
    _data.stop |= _exceptionRaised;

    broadcast(_world, _data.stop, rank_0);
    if (_world.rank() == rank_0) {
      SaveCurrentBendersData();
    }
  }

  if (_world.rank() == rank_0) {
    CloseCsvFile();
    EndWritingInOutputFile();
    write_basis();
  }
}

void BendersMpi::launch() {
  build_input_map();
  _world.barrier();

  initialize_problems();
  _world.barrier();

  run();
  _world.barrier();

  post_run_actions();

  free();
  _world.barrier();
}
