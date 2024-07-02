#include "BendersSequential.h"

#include <algorithm>
#include <iomanip>
#include <utility>

#include "Timer.h"

#include "solver_utils.h"

/*!
 *  \brief Constructor of class BendersSequential
 *
 *  Method to build a BendersSequential element, initializing each problem from
 * a list
 *
 *  \param options : set of options fixed by the user
 */

BendersSequential::BendersSequential(
    BendersBaseOptions const &options, Logger logger, Writer writer,
    std::shared_ptr<MathLoggerDriver> mathLoggerDriver)
    : BendersBase(options, std::move(logger), std::move(writer),
                  mathLoggerDriver) {}

void BendersSequential::InitializeProblems() {
  MatchProblemToId();

  reset_master<WorkerMaster>(master_variable_map_, get_master_path(),
                                get_solver_name(), get_log_level(),
                                _data.nsubproblem, solver_log_manager_,
                                IsResumeMode(), _logger);
  for (const auto &problem : coupling_map_) {
    const auto subProblemFilePath = GetSubproblemPath(problem.first);

    AddSubproblem(problem);
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
 * Method to build subproblem cuts, store them in the BendersSequential trace
 * and add them to the Master problem
 *
 */
void BendersSequential::BuildCut() {
  SubProblemDataMap subproblem_data_map;
  Timer timer;
  GetSubproblemCut(subproblem_data_map);
  SetSubproblemCost(0);
  for (const auto &[_, subproblem_data] : subproblem_data_map) {
    SetSubproblemCost(GetSubproblemCost() + subproblem_data.subproblem_cost);
  }

  SetSubproblemsWalltime(timer.elapsed());
  _data.ub = 0;
  BuildCutFull(subproblem_data_map);
}

/*!
 *  \brief Run BendersSequential algorithm
 *
 *  Method to run BendersSequential algorithm
 */
void BendersSequential::Run() {
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

    if (SwitchToIntegerMaster(_data.is_in_initial_relaxation)) {
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
    BuildCut();
    _logger->LogSubproblemsSolvingWalltime(GetSubproblemsWalltime());

    compute_ub();
    update_best_ub();

    _logger->log_at_iteration_end(bendersDataToLogData(_data));

    UpdateTrace();

    set_timer_master(timer_master.elapsed());
    _data.iteration_time = -_data.benders_time;
    _data.benders_time = GetBendersTime();
    _data.iteration_time += _data.benders_time;
    _data.stop = ShouldBendersStop();
    SaveCurrentBendersData();
  }
  CloseCsvFile();
  EndWritingInOutputFile();
  write_basis();
}

void BendersSequential::launch() {
  _logger->display_message("Building input");
  _logger->display_message("Constructing workers...");

  InitializeProblems();
  _logger->display_message("Running solver...");
  try {
    Run();
    _logger->display_message(BendersName() + " solver terminated.");
  } catch (std::exception const &ex) {
    std::string error = "Exception raised : " + std::string(ex.what());
    _logger->display_message(error);
  }

  post_run_actions();
  free();
}
