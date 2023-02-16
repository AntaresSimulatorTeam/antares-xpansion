#pragma once

#include "ArchiveReader.h"
#include "BendersBase.h"
#include "BendersStructsDatas.h"
#include "SubproblemCut.h"
#include "SubproblemWorker.h"
#include "Timer.h"
#include "Worker.h"
#include "WorkerMaster.h"
#include "common_mpi.h"
#include "core/ILogger.h"

/*!
 * \class BendersMpi
 * \brief Class use run the benders algorithm in parallel
 */
class BendersMpi : public BendersBase {
 public:
  ~BendersMpi() override = default;
  BendersMpi(BendersBaseOptions const &options, Logger logger, Writer writer,
             mpi::environment &env, mpi::communicator &world);

  void launch() override;
  std::string BendersName() const { return "Mpi"; }
  const unsigned int rank_0 = 0;

 protected:
  void free() override;
  void run() override;
  void initialize_problems() override;

 private:
  void step_1_solve_master();
  void step_2_solve_subproblems_and_build_cuts();
  void step_4_update_best_solution(int rank, const Timer &timer_master);

  void master_build_cuts(
      std::vector<SubProblemDataMap> gathered_subproblem_map);
  SubProblemDataMap get_subproblem_cut_package();

  void solve_master_and_create_trace();

  bool _exceptionRaised = false;

  void do_solve_master_create_trace_and_update_cuts();

  void broadcast_the_master_problem();

  void gather_subproblems_cut_package_and_build_cuts(
      const SubProblemDataMap &subproblem_data_map, const Timer &process_timer);

  void write_exception_message(const std::exception &ex) const;

  void check_if_some_proc_had_a_failure(int success);
  mpi::environment &_env;
  mpi::communicator &_world;

 protected:
  ArchiveReader reader_;
  [[nodiscard]] bool shouldParallelize() const final { return false; }
  void PreRunInitialization();
  int Rank() const { return _world.rank(); }
  template <typename T>
  void BroadCast(T value, int root) {
    mpi::broadcast(_world, value, root);
  }
  template <typename T>
  void Gather(const T &value, std::vector<T> &vector_of_values, int root) {
    mpi::gather(_world, value, vector_of_values, root);
  }
  void Barrier() { _world.barrier(); }
  void BuildMasterProblem();
  int WorldSize() const { return _world.size(); }
};
