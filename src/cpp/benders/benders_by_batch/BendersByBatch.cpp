#include "BendersByBatch.h"

#include "glog/logging.h"

BendersByBatch::BendersByBatch(BendersBaseOptions const &options, Logger logger,
                               Writer writer)
    : BendersBase(options, std::move(logger), std::move(writer)) {}

void BendersByBatch::launch() {
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
void BendersByBatch::free() {
  if (get_master()) {
    free_master();
  }
  free_subproblems();
}
void BendersByBatch::run() {
  set_cut_storage();
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

    push_in_trace(std::make_shared<WorkerMasterData>());

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
void BendersByBatch::initialize_problems() {
  match_problem_to_id();

  reset_master(new WorkerMaster(master_variable_map, get_master_path(),
                                get_solver_name(), get_log_level(),
                                _data.nsubproblem, log_name(), IsResumeMode()));
  for (const auto &problem : coupling_map) {
    addSubproblem(problem);
    AddSubproblemName(problem.first);
  }
}

/*!
 * \brief Build subproblem cut and store it in the BendersSequential trace
 *
 * Method to build subproblem cuts, store them in the BendersSequential trace
 * and add them to the Master problem
 *
 */
void BendersByBatch::build_cut() {
  SubproblemCutPackage subproblem_cut_package;
  AllCutPackage all_package;
  Timer timer;
  getSubproblemCut(subproblem_cut_package);
  SetSubproblemCost(0);
  for (const auto &pair_name_subproblemcutdata_l : subproblem_cut_package) {
    SetSubproblemCost(
        GetSubproblemCost() +
        pair_name_subproblemcutdata_l.second.first.second[SUBPROBLEM_COST]);
  }

  SetSubproblemTimers(timer.elapsed());
  all_package.push_back(subproblem_cut_package);
  build_cut_full(all_package);
}