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
 * Method to build subproblem cuts, store them in the BendersSequential trace
 * and add them to the Master problem
 *
 */
void BendersSequential::build_cut() {
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

std::vector<int> get_int_var_ids(SolverAbstract::Ptr &master) {
  std::vector<std::string> var_names =
      master->get_col_names(0, master->get_ncols()-1);
  std::vector<int> int_var_ids;

  for (const auto &var_name : var_names) {
    if (var_name.rfind("nbUnits_", 0) != std::string::npos) {
      std::cout << var_name << std::endl;
      int_var_ids.push_back(master->get_col_index(var_name));
    }
  }
  return int_var_ids;
}

// struct savedRows {
//   std::vector<int> mstart;
//   std::vector<int> mclind;
//   std::vector<double> dmatval;
// };

// savedRows save_int_constraints(SolverAbstract::Ptr &master,
//                           std::vector<int> int_var_ids) {
//   std::sort(int_var_ids.begin(), int_var_ids.end());
//   std::vector<int> mstart;
//   std::vector<int> mclind;
//   std::vector<double> dmatval;
//   solver_getrows(master, mstart, mclind, dmatval, int_var_ids.front(),
//                  int_var_ids.back());
//   savedRows saved_rows = {mstart, mclind, dmatval};
//   return saved_rows;
// }

void remove_integrity_constraints(SolverAbstract::Ptr &master,
                                  std::vector<int> int_var_ids) {
  std::vector<char> col_types(int_var_ids.size(), 'C');
  master->chg_col_type(int_var_ids, col_types);
}

void add_integrity_constraints(SolverAbstract::Ptr &master, std::vector<int> int_var_ids) {
  std::vector<char> col_types(int_var_ids.size(), 'I');
  master->chg_col_type(int_var_ids, col_types);
}

/*!
 *  \brief Run BendersSequential algorithm
 *
 *  Method to run BendersSequential algorithm
 */
void BendersSequential::run() {
  set_cut_storage();
  init_data();
  ChecksResumeMode();
  if (is_trace()) {
    OpenCsvFile();
  }

  bool relaxed_master = true;
  std::vector<int> int_var_ids =
      get_int_var_ids(get_master()->_solver);
  remove_integrity_constraints(get_master()->_solver, int_var_ids);

  while (!_data.stop) {
    Timer timer_master;
    ++_data.it;

    if (relaxed_master &&
        _data.lb + 1000 * _options.ABSOLUTE_GAP >= _data.best_ub) {
      relaxed_master = false;
      std::cout << "switch to integer" << std::endl;
      add_integrity_constraints(get_master()->_solver, int_var_ids);
    }
    
    _logger->log_at_initialization(_data.it + GetNumIterationsBeforeRestart());
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
    _data.elapsed_time = GetBendersTime();
    _data.stop = stopping_criterion();
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
    LOG(INFO) << "BendersSequential solver terminated." << std::endl;
  } catch (std::exception const &ex) {
    std::string error = "Exception raised : " + std::string(ex.what());
    LOG(WARNING) << error << std::endl;
    _logger->display_message(error);
  }

  post_run_actions();
  free();
}
