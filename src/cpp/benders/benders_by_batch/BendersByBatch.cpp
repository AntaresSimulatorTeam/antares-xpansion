#include "BendersByBatch.h"

#include <mutex>

#include "BatchCollection.h"
#include "RandomBatchShuffler.h"
#include "glog/logging.h"

BendersByBatch::BendersByBatch(BendersBaseOptions const &options, Logger logger,
                               Writer writer, mpi::environment &env,
                               mpi::communicator &world)
    : BendersMpi(options, std::move(logger), std::move(writer), env, world) {}

void BendersByBatch::run() {
  PreRunInitialization();

  size_t batch_size = 0;
  BatchCollection batch_collection;
  if (rank() == rank_0) {
    batch_size = Options().BATCH_SIZE == 0 ? GetSubProblemNames().size()
                                           : Options().BATCH_SIZE;

    batch_collection.SetLogger(_logger);
    batch_collection.SetBatchSize(batch_size);
    batch_collection.SetSubProblemNames(GetSubProblemNames());
    batch_collection.BuildBatches();
    BroadCast(batch_size, rank_0);
  }
  auto number_of_batch = batch_collection.NumberOfBatch();
  unsigned batch_counter = 0;

  auto current_batch_id = 0;
  int number_of_sub_problem_resolved = 0;

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

    auto remaining_epsilon = AbsoluteGap();

    random_batch_permutation_ = RandomBatchShuffler(number_of_batch)
                                    .GetCyclicBatchOrder(current_batch_id);
    batch_counter = 0;

    while (batch_counter < number_of_batch) {
      current_batch_id = random_batch_permutation_[batch_counter];
      const auto &batch = batch_collection.GetBatchFromId(current_batch_id);
      const auto &batch_sub_problems = batch.sub_problem_names;
      double sum = 0;
      build_cut(batch_sub_problems, &sum);
      number_of_sub_problem_resolved += batch_sub_problems.size();
      remaining_epsilon -= sum;
      UpdateTrace();

      if (remaining_epsilon > 0)
        batch_counter++;
      else
        break;
    }
    _logger->number_of_sub_problem_resolved(number_of_sub_problem_resolved);
  }
  compute_ub();
  update_best_ub();
  _logger->log_at_iteration_end(bendersDataToLogData(_data));
  SaveCurrentBendersData();

  CloseCsvFile();
  EndWritingInOutputFile();
  write_basis();
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
  SubProblemDataMap subproblem_data_map;
  // AllCutPackage all_package;
  Timer timer;
  getSubproblemCut(subproblem_data_map, batch_sub_problems, sum);

  for (const auto &sub_problem_name : batch_sub_problems) {
    auto sub_problem_data = subproblem_data_map[sub_problem_name];
    SetSubproblemCost(GetSubproblemCost() + sub_problem_data.subproblem_cost);
  }

  SetSubproblemTimers(timer.elapsed());
  // all_package.push_back(subproblem_data_map);
  build_cut_full(subproblem_data_map);
}

/*!
 *  \brief Solve and store optimal variables of all Subproblem Problems
 *
 *  Method to solve and store optimal variables of all Subproblem Problems
 * after fixing trial values
 *
 *  \param subproblem_data_map : map storing for each subproblem its cut
 */
void BendersByBatch::getSubproblemCut(
    SubProblemDataMap &subproblem_data_map,
    const std::vector<std::string> &batch_sub_problems, double *sum) const {
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
      [this, &nameAndWorkers, &m, &subproblem_data_map, &sum](auto &policy) {
        std::for_each(
            policy, nameAndWorkers.begin(), nameAndWorkers.end(),
            [this, &m, &subproblem_data_map,
             &sum](const std::pair<std::string, SubproblemWorkerPtr> &kvp) {
              const auto &[name, worker] = kvp;
              Timer subproblem_timer;
              SubProblemData subproblem_data;
              worker->fix_to(_data.x_cut);
              worker->solve(subproblem_data.lpstatus, Options().OUTPUTROOT,
                            Options().LAST_MASTER_MPS + MPS_SUFFIX);
              worker->get_value(
                  subproblem_data.subproblem_cost);  // solution phi(x,s)
              worker->get_subgradient(
                  subproblem_data.var_name_and_subgradient);  // dual pi_s
              *sum += subproblem_data.subproblem_cost -
                      GetAlpha_i()[ProblemToId(name)];
              worker->get_splex_num_of_ite_last(subproblem_data.simplex_iter);
              subproblem_data.subproblem_timer = subproblem_timer.elapsed();
              std::lock_guard guard(m);
              subproblem_data_map[name] = subproblem_data;
            });
      },
      shouldParallelize());
}
// void BendersByBatch::launch() {
//   build_input_map();

//   LOG(INFO) << "Building input" << std::endl;

//   LOG(INFO) << "Constructing workers..." << std::endl;

//   initialize_problems();
//   LOG(INFO) << "Running solver..." << std::endl;
//   try {
//     run();
//     LOG(INFO) << BendersName() + " solver terminated." << std::endl;
//   } catch (std::exception const &ex) {
//     std::string error = "Exception raised : " + std::string(ex.what());
//     LOG(WARNING) << error << std::endl;
//     _logger->display_message(error);
//   }

//   post_run_actions();
//   free();
// }
