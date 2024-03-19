#pragma once

#include "ArchiveReader.h"
#include "BendersBase.h"
#include "BendersStructsDatas.h"
#include "ILogger.h"
#include "SubproblemCut.h"
#include "SubproblemWorker.h"
#include "Timer.h"
#include "Worker.h"
#include "WorkerMaster.h"
#include "common_mpi.h"

/*!
 * \class BendersMpi
 * \brief Class use run the benders algorithm in parallel
 */
class BendersMpi : public BendersBase {
 public:
  ~BendersMpi() override = default;
  BendersMpi(BendersBaseOptions const &options, Logger logger, Writer writer,
             mpi::environment &env, mpi::communicator &world,
             std::shared_ptr<MathLoggerDriver> mathLoggerDriver);

  void launch() override;
  virtual std::string BendersName() const { return "Benders mpi"; }
  const unsigned int rank_0 = 0;

 protected:
  void free() override;
  void Run() override;
  void InitializeProblems() override;
  void BroadcastXCut();

 private:
  void step_1_solve_master();
  void step_2_solve_subproblems_and_build_cuts();
  void step_4_update_best_solution(int rank);

  void master_build_cuts(
      std::vector<SubProblemDataMap> gathered_subproblem_map);
  SubProblemDataMap get_subproblem_cut_package();

  void solve_master_and_create_trace();

  bool _exceptionRaised = false;

  void do_solve_master_create_trace_and_update_cuts();

  void gather_subproblems_cut_package_and_build_cuts(
      const SubProblemDataMap &subproblem_data_map, const Timer &process_timer);

  void write_exception_message(const std::exception &ex) const;

  void check_if_some_proc_had_a_failure(int success);
  mpi::environment &_env;
  mpi::communicator &_world;

 protected:
  [[nodiscard]] bool shouldParallelize() const final { return false; }
  void PreRunInitialization();
  int Rank() const { return _world.rank(); }
  template <typename T>
  void BroadCast(T &value, int root) const {
    mpi::broadcast(_world, value, root);
  }
  template <typename T>
  void BroadCast(T *values, int n, int root) const {
    mpi::broadcast(_world, values, n, root);
  }
  template <typename T>
  void Gather(const T &value, std::vector<T> &vector_of_values,
              int root) const {
    mpi::gather(_world, value, vector_of_values, root);
  }
  void BuildMasterProblem();
  int WorldSize() const { return _world.size(); }
  void Barrier() const { _world.barrier(); }

  template <typename T, typename Op>
  void Reduce(const T &in_value, T &out_value, Op op, int root) const {
    mpi::reduce(_world, in_value, out_value, op, root);
  }
  template <typename T, typename Op>
  void AllReduce(const T &in_value, T &out_value, Op op) const {
    mpi::all_reduce(_world, in_value, out_value, op);
  }
  virtual double ComputeSubproblemsContributionToOuterLoopCriterion(
      const SubProblemDataMap &subproblem_data_map);
};
