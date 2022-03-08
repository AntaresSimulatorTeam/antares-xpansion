#pragma once

#include "BendersBase.h"
#include "BendersStructsDatas.h"
#include "SlaveCut.h"
#include "Timer.h"
#include "Worker.h"
#include "WorkerMaster.h"
#include "WorkerSlave.h"
#include "common_mpi.h"
#include "core/ILogger.h"

/*!
 * \class BendersMpi
 * \brief Class use run the benders algorithm in parallel
 */
class BendersMpi : public BendersBase {
 public:
  virtual ~BendersMpi() = default;
  BendersMpi(BendersBaseOptions const &options, Logger &logger, Writer writer,
             mpi::environment &env, mpi::communicator &world);

  void load();
  virtual void launch();

 protected:
  virtual void free();
  virtual void run();
  virtual void initialize_problems();

 private:
  void step_1_solve_master();
  void step_2_solve_slaves_and_build_cuts();
  void step_4_update_best_solution(int rank, const Timer &timer_master,
                                   const Timer &benders_timer);

  void master_build_cuts(AllCutPackage all_package);
  SlaveCutPackage get_slave_package();

  void solve_master_and_create_trace();

  bool _exceptionRaised = false;

  void do_solve_master_create_trace_and_update_cuts();

  void broadcast_the_master_problem();

  void gather_slave_cut_package_and_build_cuts(
      const SlaveCutPackage &slave_cut_package, const Timer &timer_slaves);

  void write_exception_message(const std::exception &ex);

  void check_if_some_proc_had_a_failure(int success);
  mpi::environment &_env;
  mpi::communicator &_world;
  const unsigned int rank_0 = 0;
};
