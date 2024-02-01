#include "OuterLoopCriterion.h"

OuterLoop::OuterLoop(std::shared_ptr<IOuterLoopCriterion> criterion,
                     std::shared_ptr<IMasterUpdate> master_updater,
                     pBendersBase benders)
    : criterion_(std::move(criterion)),
      master_updater_(std::move(master_updater)),
      benders_(std::move(benders)) {}

void OuterLoop::Run() {
  // master_updater_->
}