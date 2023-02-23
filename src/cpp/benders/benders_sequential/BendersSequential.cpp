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
                                     Logger logger, Writer writer)
    : BendersBase(options, std::move(logger), std::move(writer)) {}

void BendersSequential::initialize_problems() {
  match_problem_to_id();

  reset_master(new WorkerMaster(master_variable_map, get_master_path(),
                                get_solver_name(), get_log_level(),
                                _data.nsubproblem, log_name(), IsResumeMode()));
  for (const auto &problem : coupling_map) {
    const auto subProblemFilePath = GetSubproblemPath(problem.first);

    addSubproblem(problem);
    AddSubproblemName(problem.first);
    std::filesystem::remove(subProblemFilePath);
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
 * Method to build subproblem cuts, store them in the BendersSequential trace
 * and add them to the Master problem
 *
 */
void BendersSequential::build_cut() {
  SubProblemDataMap subproblem_data_map;
  Timer timer;
  getSubproblemCut(subproblem_data_map);
  SetSubproblemCost(0);
  for (const auto &[_, subproblem_data] : subproblem_data_map) {
    SetSubproblemCost(GetSubproblemCost() + subproblem_data.subproblem_cost);
  }

  SetSubproblemTimers(timer.elapsed());
  _data.ub = 0;
  build_cut_full(subproblem_data_map);
}

/*!
 *  \brief Run BendersSequential algorithm
 *
 *  Method to run BendersSequential algorithm
 */
void BendersSequential::run() {
  init_data();
  ChecksResumeMode();
  if (is_trace()) {
    OpenCsvFile();
  }

  if (is_initial_relaxation_requested()) {
    _logger->LogAtInitialRelaxation();
    DeactivateIntegrityConstraints();
    SetDataPreRelaxation();
  }

  while (!_data.stop) {
    Timer timer_master;
    ++_data.it;

    if (switch_to_integer_master(_data.is_in_initial_relaxation)) {
      _logger->LogAtSwitchToInteger();
      ActivateIntegrityConstraints();
      ResetDataPostRelaxation();
    }

    _logger->log_at_initialization(_data.it + GetNumIterationsBeforeRestart());
    _logger->display_message("\tSolving master...");
    get_master_value();
    _logger->log_master_solving_duration(get_timer_master());

    ComputeXCut();
    _logger->log_iteration_candidates(bendersDataToLogData(_data));

    _logger->display_message("\tSolving subproblems...");
    build_cut();
    _logger->log_subproblems_solving_duration(GetSubproblemTimers());

    compute_ub();
    update_best_ub();

    _logger->log_at_iteration_end(bendersDataToLogData(_data));

    UpdateTrace();

    set_timer_master(timer_master.elapsed());
    _data.elapsed_time = GetBendersTime();
    _data.stop = ShouldBendersStop();
    SaveCurrentBendersData();
  }
  CloseCsvFile();
  EndWritingInOutputFile();
  write_basis();
}

void BendersSequential::launch() {
  build_input_map();

  LOG(INFO) << "Building input" << std::endl;

  LOG(INFO) << "Constructing workers..." << std::endl;

  initialize_problems();
  LOG(INFO) << "Running solver..." << std::endl;
  try {
    run();
    LOG(INFO) << BendersName() + " solver terminated." << std::endl;
  } catch (std::exception const &ex) {
    std::string error = "Exception raised : " + std::string(ex.what());
    LOG(WARNING) << error << std::endl;
    _logger->display_message(error);
  }

  post_run_actions();
  free();
}
