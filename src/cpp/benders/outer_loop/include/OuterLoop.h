#pragma once
#include "CutsManagement.h"
#include "MasterUpdate.h"

namespace Outerloop {
class IOuterLoop {
 public:
  explicit IOuterLoop(std::shared_ptr<IMasterUpdate> master_updater,
                      std::shared_ptr<ICutsManager> cuts_manager);
  virtual void Run() = 0;
  virtual void OuterLoopCheckFeasibility() = 0;
  virtual void OuterLoopBilevelChecks() = 0;

 private:
  std::shared_ptr<IMasterUpdate> master_updater_;
  std::shared_ptr<ICutsManager> cuts_manager_;
};

}  // namespace Outerloop