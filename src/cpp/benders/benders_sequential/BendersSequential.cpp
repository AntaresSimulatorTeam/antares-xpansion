#include "BendersSequential.h"

#include <algorithm>
#include <iomanip>
#include <utility>

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
    : BendersBase(options, logger, std::move(writer)) {}

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
 * \brief Build subproblem cut and store it in the BendersSequential trace
 *
 * Method to build subproblem cuts, store them in the BendersSequential trace and
 * add them to the Master problem
 *
 */
void BendersSequential::build_cut() {
  SubproblemCutPackage subproblem_cut_package;
  AllCutPackage all_package;
  Timer timer;
  getSubproblemCut(subproblem_cut_package);
  SetSubproblemCost(0);
  for (const auto& pair_name_subproblemcutdata_l : subproblem_cut_package) {
    SetSubproblemCost(
        GetSubproblemCost() +
        pair_name_subproblemcutdata_l.second.first.second[SUBPROBLEM_COST]);
  }

  SetSubproblemTimers(timer.elapsed());
  all_package.push_back(subproblem_cut_package);
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

    push_in_trace(std::make_shared<WorkerMasterData>());

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
  save_basis(get_basis());

  // std::ofstream
  // row_basis("/home/bittartho/Documents/xpansion/etudes/sensitivity_test_case/row_basis.csv");
  // std::copy(optimal_basis.first.rbegin(), optimal_basis.first.rend(),
  //         std::ostream_iterator<int>(row_basis, "\n"));

  // std::ofstream
  // col_basis("/home/bittartho/Documents/xpansion/etudes/sensitivity_test_case/col_basis.csv");
  // std::copy(optimal_basis.second.rbegin(), optimal_basis.second.rend(),
  //         std::ostream_iterator<int>(col_basis, "\n"));

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
  } catch (std::exception const &ex) {
    std::string error = "Exception raised : " + std::string(ex.what());
    LOG(WARNING) << error << std::endl;
    _logger->display_message(error);
  }

  post_run_actions();
  free();
}
