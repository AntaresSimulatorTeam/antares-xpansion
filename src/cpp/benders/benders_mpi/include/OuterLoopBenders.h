#pragma once
#include "BendersBase.h"
#include "CutsManagement.h"
#include "IMasterUpdate.h"
#include "IOuterLoop.h"
#include "common_mpi.h"

namespace Outerloop {
class OuterLoopBenders : public IOuterLoop {
 public:
  explicit OuterLoopBenders(std::shared_ptr<IMasterUpdate> master_updater,
                            std::shared_ptr<ICutsManager> cuts_manager,
                            pBendersBase benders, mpi::environment& env,
                            mpi::communicator& world);
  void Run() override;
  void OuterLoopCheckFeasibility() override;
  void OuterLoopBilevelChecks() override;

 private:
  void PrintLog();
  std::shared_ptr<IMasterUpdate> master_updater_;
  std::shared_ptr<ICutsManager> cuts_manager_;
  pBendersBase benders_;
  BendersLoggerBase loggers_;
  mpi::environment& env_;
  mpi::communicator& world_;
};
}  // namespace Outerloop