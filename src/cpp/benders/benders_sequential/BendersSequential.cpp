#include "BendersSequential.h"

#include <algorithm>
#include <iomanip>

#include "Timer.h"
#include "glog/logging.h"
#include "solver_utils.h"

/*!
 *  \brief Constructor of class BendersSequential
 *
 *  Method to build a BendersSequential element, initializing each problem from
 * a list
 *
 *  \param options : set of options fixed by the user
 */

BendersSequential::BendersSequential(BendersBaseOptions const &options,
                                     Logger &logger, Writer writer)
    : BendersBase(options, logger, writer) {}

void BendersSequential::initialize_problems() {
  match_problem_to_id();

  reset_master(new WorkerMaster(master_variable_map, get_master_path(),
                                get_solver_name(), get_log_level(),
                                _data.nsubproblem, log_name()));
  for (const auto &problem : coupling_map) {
    addSubproblem(problem);
    AddSubproblemName(problem.first);
  }
}

/*!
 *  \brief Method to free the memory used by each problem
 */
void BendersSequential::free() {
  if (get_master()) {
    free_master();
  }
  free_subproblems();
}

/*!
 *  \brief Build Slave cut and store it in the BendersSequential trace
 *
 *  Method to build Slaves cuts, store them in the BendersSequential trace and
 * add them to the Master problem
 *
 */
void BendersSequential::build_cut() {
  SubproblemCutPackage slave_cut_package;
  AllCutPackage all_package;
  Timer timer_slaves;
  getSubproblemCut(slave_cut_package);
  SetSubproblemCost(0);
  for (auto pairSlavenameSlavecutdata_l : slave_cut_package) {
    SetSubproblemCost(
        GetSubproblemCost() +
        pairSlavenameSlavecutdata_l.second.first.second[SUBPROBLEM_COST]);
  }

  SetSubproblemTimers(timer_slaves.elapsed());
  all_package.push_back(slave_cut_package);
  build_cut_full(all_package);
}

/*!
 *  \brief Run BendersSequential algorithm
 *
 *  Method to run BendersSequential algorithm
 */
void BendersSequential::run() {
  set_cut_storage();
  init_data();
  Timer benders_timer;
  while (!_data.stop) {
    Timer timer_master;
    ++_data.it;

    _logger->log_at_initialization(bendersDataToLogData(_data));
    _logger->display_message("\tSolving master...");
    get_master_value();
    _logger->log_master_solving_duration(get_timer_master());

    _logger->log_iteration_candidates(bendersDataToLogData(_data));

    push_in_trace(WorkerMasterDataPtr(new WorkerMasterData));

    _logger->display_message("\tSolving subproblems...");
    build_cut();
    _logger->log_subproblems_solving_duration(GetSubproblemTimers());

    update_best_ub();

    _logger->log_at_iteration_end(bendersDataToLogData(_data));

    update_trace();

    set_timer_master(timer_master.elapsed());
    _data.elapsed_time = benders_timer.elapsed();
    _data.stop = stopping_criterion();
  }

  if (is_trace()) {
    print_csv();
  }
}

void BendersSequential::launch() {
  build_input_map();

  LOG(INFO) << "Building input" << std::endl;

  LOG(INFO) << "Constructing workers..." << std::endl;

  initialize_problems();
  LOG(INFO) << "Running solver..." << std::endl;
  try {
    run();
    LOG(INFO) << "BendersSequential solver terminated." << std::endl;
  } catch (std::exception &ex) {
    std::string error = "Exception raised : " + std::string(ex.what());
    LOG(WARNING) << error << std::endl;
    _logger->display_message(error);
  }

  post_run_actions();
  free();
}
