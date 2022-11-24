#pragma once

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
  BendersMpi(BendersBaseOptions const &options, Logger &logger, Writer writer,
             mpi::environment &env, mpi::communicator &world);

  void launch() override;

 protected:
  void free() override;
  void run() override;
  void initialize_problems() override;

 private:
  void step_1_solve_master();
  void step_2_solve_subproblems_and_build_cuts();
  void step_4_update_best_solution(int rank, const Timer &timer_master);

  void master_build_cuts(std::vector<SubProblemDataMap> gather_subproblem_map);
  SubProblemDataMap get_subproblem_cut_package();

  void solve_master_and_create_trace();

  bool _exceptionRaised = false;

  void do_solve_master_create_trace_and_update_cuts();

  void broadcast_the_master_problem();

  void gather_subproblems_cut_package_and_build_cuts(
      const SubProblemDataMap &subproblem_data_map, const Timer &process_timer);

  void write_exception_message(const std::exception &ex) const;

  void check_if_some_proc_had_a_failure(int success);
  [[nodiscard]] bool shouldParallelize() const final { return false; }

  mpi::environment &_env;
  mpi::communicator &_world;
  const unsigned int rank_0 = 0;
};
