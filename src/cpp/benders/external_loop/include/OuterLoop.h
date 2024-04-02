#pragma once
#include "CutsManagement.h"
#include "MasterUpdate.h"
#include "OuterLoopCriterion.h"
#include "common_mpi.h"

namespace Outerloop {
class OuterLoop {
 public:
  explicit OuterLoop(std::shared_ptr<IOuterLoopCriterion> criterion,
                     std::shared_ptr<IMasterUpdate> master_updater,
                     std::shared_ptr<ICutsManager> cuts_manager,
                     pBendersBase benders, mpi::environment& env,
                     mpi::communicator& world);
  void Run();

 private:
  void PrintLog();
  std::shared_ptr<IOuterLoopCriterion> criterion_;
  std::shared_ptr<IMasterUpdate> master_updater_;
  std::shared_ptr<ICutsManager> cuts_manager_;
  pBendersBase benders_;
  BendersLoggerBase loggers_;
  mpi::environment& env_;
  mpi::communicator& world_;
};

}  // namespace Outerloop