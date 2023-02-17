#include "BendersByBatch.h"

#include <algorithm>
#include <mutex>
#include <numeric>

#include "BatchCollection.h"
#include "RandomBatchShuffler.h"
#include "glog/logging.h"

BendersByBatch::BendersByBatch(BendersBaseOptions const &options, Logger logger,
                               Writer writer, mpi::environment &env,
                               mpi::communicator &world)
    : BendersMpi(options, logger, writer, env, world) {}

void BendersByBatch::initialize_problems() {
  match_problem_to_id();

  BuildMasterProblem();
  const auto &coupling_map_size = coupling_map.size();
  std::vector<std::string> problem_names(coupling_map_size);
  for (const auto &[problem_name, _] : coupling_map) {
    problem_names.emplace_back(problem_name);
  }
  auto batch_size =
      Options().BATCH_SIZE == 0 ? coupling_map_size : Options().BATCH_SIZE;
  batch_collection_.SetLogger(_logger);
  batch_collection_.SetBatchSize(batch_size);
  batch_collection_.SetSubProblemNames(problem_names);
  batch_collection_.BuildBatches();
  BroadCast(batch_collection_, rank_0);
  // Dispatch subproblems to process
  auto problem_count = 0;
  for (const auto &batch : batch_collection_.BatchCollections()) {
    for (const auto &problem_name : batch.sub_problem_names) {
      // In case there are more subproblems than process
      if (auto process_to_feed = problem_count % WorldSize();
          process_to_feed ==
          Rank()) {  // Assign  [problemNumber % processCount] to processID

        const auto subProblemFilePath = GetSubproblemPath(problem_name);
        if (Options().MPS_IN_ZIP)
          reader_.ExtractFile(subProblemFilePath.filename());
        addSubproblem({problem_name, coupling_map[problem_name]});
        AddSubproblemName(problem_name);
        std::filesystem::remove(subProblemFilePath);
        problem_count++;
      }
    }
  }
}
void BendersByBatch::run() {
  PreRunInitialization();

  // if (Rank() == rank_0) {
  // }
  auto number_of_batch = batch_collection_.NumberOfBatch();
  unsigned batch_counter = 0;

  auto current_batch_id = 0;
  int number_of_sub_problem_resolved = 0;

  while (batch_counter < number_of_batch) {
    _data.it++;
    _data.ub = 0;
    SetSubproblemCost(0);
    auto remaining_epsilon = AbsoluteGap();

    if (Rank() == rank_0) {
      if (switch_to_integer_master(_data.is_in_initial_relaxation)) {
        _logger->LogAtSwitchToInteger();
        ActivateIntegrityConstraints();
        ResetDataPostRelaxation();
      }

      _logger->log_at_initialization(_data.it +
                                     GetNumIterationsBeforeRestart());
      _logger->display_message("\tSolving master...");
      get_master_value();
      _logger->log_master_solving_duration(get_timer_master());

      ComputeXCut();
      _logger->log_iteration_candidates(bendersDataToLogData(_data));

      random_batch_permutation_ = RandomBatchShuffler(number_of_batch)
                                      .GetCyclicBatchOrder(current_batch_id);
    }
    BroadCast(random_batch_permutation_, rank_0);
    // }
    batch_counter = 0;

    while (batch_counter < number_of_batch) {
      current_batch_id = random_batch_permutation_[batch_counter];
      const auto &batch = batch_collection_.GetBatchFromId(current_batch_id);
      const auto &batch_sub_problems = batch.sub_problem_names;
      double sum = 0;
      build_cut(batch_sub_problems, &sum);
      std::vector<double> vect_of_sum;
      Gather(sum, vect_of_sum, rank_0);
      // Barrier();
      if (Rank() == rank_0) {
        number_of_sub_problem_resolved += batch_sub_problems.size();

        remaining_epsilon -=
            std::reduce(vect_of_sum.begin(), vect_of_sum.end());
        UpdateTrace();

        if (remaining_epsilon > 0) {
          batch_counter++;
        } else
          break;
      }
      BroadCast(batch_counter, rank_0);
      // }
      // Barrier();
    }
    // if (Rank() == rank_0) {
    _logger->number_of_sub_problem_resolved(number_of_sub_problem_resolved);
    // }
  }
  if (Rank() == rank_0) {
    compute_ub();
    update_best_ub();
    _logger->log_at_iteration_end(bendersDataToLogData(_data));
    SaveCurrentBendersData();

    CloseCsvFile();
    EndWritingInOutputFile();
    write_basis();
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
  SubProblemDataMap subproblem_data_map;
  // AllCutPackage all_package;
  Timer timer;
  getSubproblemCut(subproblem_data_map, batch_sub_problems, sum);

  for (const auto &sub_problem_name : batch_sub_problems) {
    if (subproblem_data_map.find(sub_problem_name) !=
        subproblem_data_map.end()) {
      auto sub_problem_data = subproblem_data_map[sub_problem_name];
      SetSubproblemCost(GetSubproblemCost() + sub_problem_data.subproblem_cost);
    }
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
    //   nameAndWorkers.emplace_back(name, sub_pblm_map[name]);
    std::copy_if(
        sub_pblm_map.cbegin(), sub_pblm_map.cend(),
        std::back_inserter(nameAndWorkers),
        [&sub_pblm_map, &name](std::pair<std::string, SubproblemWorkerPtr>
                                   name_subproblemWorkerPtr) {
          return sub_pblm_map.find(name) != sub_pblm_map.cend();
        });
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
