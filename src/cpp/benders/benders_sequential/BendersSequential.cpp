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

void BendersSequential::initialise_problems() {
  const auto input_ = get_input();
  if (!input_.empty()) {
    _data.nslaves = get_slaves_number();
    if (_data.nslaves < 0) {
      _data.nslaves = input_.size() - 1;
    }

    auto it(input_.begin());

    auto const it_master = input_.find(get_master_name());
    Str2Int const &master_variable(it_master->second);
    for (int i(0); i < _data.nslaves; ++it) {
      if (it != it_master) {
        set_problem_to_id(it->first, i);
        set_slave(*it);
        add_slave_name(it->first);
        i++;
      }
    }
    reset_master(new WorkerMaster(master_variable, get_master_path(),
                                  get_solver_name(), get_log_level(),
                                  _data.nslaves, log_name()));
  }
}

/*!
 *  \brief Method to free the memory used by each problem
 */
void BendersSequential::free() {
  if (get_master()) {
    free_master();
  }
  free_slaves();
}

/*!
 *  \brief Build Slave cut and store it in the BendersSequential trace
 *
 *  Method to build Slaves cuts, store them in the BendersSequential trace and
 * add them to the Master problem
 *
 */
void BendersSequential::build_cut() {
  SlaveCutPackage slave_cut_package;
  AllCutPackage all_package;
  Timer timer_slaves;
  get_slave_cut(slave_cut_package);
  set_slave_cost(0);
  for (auto pairSlavenameSlavecutdata_l : slave_cut_package) {
    set_slave_cost(get_slave_cost() +
                   pairSlavenameSlavecutdata_l.second.first.second[SLAVE_COST]);
  }

  set_timer_slaves(timer_slaves.elapsed());
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
    _logger->log_subproblems_solving_duration(get_timer_slaves());

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

  initialise_problems();
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
