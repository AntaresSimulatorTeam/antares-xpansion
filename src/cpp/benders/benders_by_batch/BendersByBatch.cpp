#include "BendersByBatch.h"

#include <mutex>

#include "BatchCollection.h"
#include "RandomBatchShuffler.h"
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
    LOG(INFO) << "Benders by batch solver terminated." << std::endl;
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

  unsigned batch_size = Options().BATCH_SIZE == 0 ? GetSubProblemNames().size()
                                                  : Options().BATCH_SIZE;

  double remaining_epsilon = AbsoluteGap();
  const auto batch_collection =
      BatchCollection(GetSubProblemNames(), batch_size);
  auto number_of_batch = batch_collection.NumberOfBatch();
  unsigned batch_counter = 0;

  auto current_batch_id = 0;
  while (batch_counter < number_of_batch) {
    _data.it++;
    _data.ub = 0;
    SetSubproblemCost(0);
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

    remaining_epsilon = AbsoluteGap();

    random_batch_permutation_ = RandomBatchShuffler(number_of_batch)
                                    .GetCyclicBatchOrder(current_batch_id);
    batch_counter = 0;
    while (batch_counter < number_of_batch) {
      current_batch_id = random_batch_permutation_[batch_counter];
      auto batch = batch_collection.GetBatchFromId(current_batch_id);
      auto batch_sub_problems = batch.sub_problem_names;
      double sum = 0;
      build_cut(batch_sub_problems, &sum);
      remaining_epsilon -= sum;
      UpdateTrace();

      if (remaining_epsilon > 0)
        batch_counter++;
      else
        break;
    }
  }
  compute_ub();
  update_best_ub();
  _logger->log_at_iteration_end(bendersDataToLogData(_data));
  SaveCurrentBendersData();

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
void BendersByBatch::build_cut(
    const std::vector<std::string> &batch_sub_problems, double *sum) {
  SubproblemCutPackage subproblem_cut_package;
  AllCutPackage all_package;
  Timer timer;
  getSubproblemCut(subproblem_cut_package, batch_sub_problems, sum);

  for (const auto &sub_problem_name : batch_sub_problems) {
    auto sub_problem_cut_data = subproblem_cut_package[sub_problem_name];
    auto sub_problem_cut_cost =
        sub_problem_cut_data.first.second[SUBPROBLEM_COST];
    SetSubproblemCost(GetSubproblemCost() + sub_problem_cut_cost);
  }

  SetSubproblemTimers(timer.elapsed());
  all_package.push_back(subproblem_cut_package);
  build_cut_full(all_package);
}

/*!
 *  \brief Solve and store optimal variables of all Subproblem Problems
 *
 *  Method to solve and store optimal variables of all Subproblem Problems
 * after fixing trial values
 *
 *  \param subproblem_cut_package : map storing for each subproblem its cut
 */
void BendersByBatch::getSubproblemCut(
    SubproblemCutPackage &subproblem_cut_package,
    const std::vector<std::string> &batch_sub_problems, double *sum) {
  *sum = 0;
  // With gcc9 there was no parallelisation when iterating on the map directly
  // so with project it in a vector
  std::vector<std::pair<std::string, SubproblemWorkerPtr>> nameAndWorkers;
  nameAndWorkers.reserve(batch_sub_problems.size());
  auto sub_pblm_map = GetSubProblemMap();
  for (const auto &name : batch_sub_problems) {
    nameAndWorkers.emplace_back(name, sub_pblm_map[name]);
  }
  std::mutex m;
  selectPolicy(
      [this, &nameAndWorkers, &m, &subproblem_cut_package, &sum](auto &policy) {
        std::for_each(
            policy, nameAndWorkers.begin(), nameAndWorkers.end(),
            [this, &m, &subproblem_cut_package,
             &sum](const std::pair<std::string, SubproblemWorkerPtr> &kvp) {
              const auto &[name, worker] = kvp;
              Timer subproblem_timer;
              auto subproblem_cut_data(std::make_shared<SubproblemCutData>());
              auto handler(std::make_shared<SubproblemCutDataHandler>(
                  subproblem_cut_data));
              worker->fix_to(_data.x_cut);
              worker->solve(handler->get_int(LPSTATUS), Options().OUTPUTROOT,
                            Options().LAST_MASTER_MPS + MPS_SUFFIX);
              worker->get_value(
                  handler->get_dbl(SUBPROBLEM_COST));  // solution phi(x,s)
              worker->get_subgradient(handler->get_subgradient());  // dual pi_s
              *sum += handler->get_dbl(SUBPROBLEM_COST) -
                      GetAlpha_i()[ProblemToId(name)];
              worker->get_splex_num_of_ite_last(handler->get_int(SIMPLEXITER));
              handler->get_dbl(SUBPROBLEM_TIMER) = subproblem_timer.elapsed();
              std::lock_guard guard(m);
              subproblem_cut_package[name] = *subproblem_cut_data;
            });
      },
      shouldParallelize());
}
