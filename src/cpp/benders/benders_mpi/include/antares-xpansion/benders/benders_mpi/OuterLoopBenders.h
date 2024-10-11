#pragma once
#include "BendersBase.h"
#include "CutsManagement.h"
#include "antares-xpansion/benders/outer_loop/IMasterUpdate.h"
#include "antares-xpansion/benders/outer_loop/OuterLoop.h"
#include "common_mpi.h"

namespace Outerloop {

class CriterionCouldNotBeSatisfied
    : public LogUtils::XpansionError<std::runtime_error> {
  using LogUtils::XpansionError<std::runtime_error>::XpansionError;
};

class OuterLoopBenders : public OuterLoop {
 public:
  explicit OuterLoopBenders(CriterionComputation& criterion_computation,
                            std::shared_ptr<IMasterUpdate> master_updater,
                            std::shared_ptr<ICutsManager> cuts_manager,
                            pBendersBase benders, mpi::communicator& world);
  //  void Run() override;
  void OuterLoopCheckFeasibility() override;
  void OuterLoopBilevelChecks() override;
  void RunAttachedAlgo() override;
  void PrintLog() override;
  void init_data() override;
  bool isExceptionRaised() override;
  double OuterLoopLambdaMin() const override;
  double OuterLoopLambdaMax() const override;
  bool UpdateMaster() override;
  ~OuterLoopBenders() override;

 private:
  std::shared_ptr<IMasterUpdate> master_updater_;
  std::shared_ptr<ICutsManager> cuts_manager_;
  pBendersBase benders_;
  BendersLoggerBase loggers_;
  mpi::communicator& world_;
  bool is_bilevel_check_all_ = false;
  void InitExternalValues(bool is_bilevel_check_all, double lambda);
};
}  // namespace Outerloop