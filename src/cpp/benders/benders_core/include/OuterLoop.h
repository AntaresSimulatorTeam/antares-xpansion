#pragma once
#include "MasterUpdate.h"
#include "OuterLoopCriterion.h"

class OuterLoop {
 public:
  explicit OuterLoop(std::shared_ptr<IOuterLoopCriterion> criterion,
                     std::shared_ptr<IMasterUpdate> master_updater,
                     pBendersBase benders);
  void Run();

 private:
  std::shared_ptr<IOuterLoopCriterion> criterion_;
  std::shared_ptr<IMasterUpdate> master_updater_;
  pBendersBase benders_;
};