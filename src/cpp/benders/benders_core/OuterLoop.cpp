#include "OuterLoop.h"

OuterLoop::OuterLoop(std::shared_ptr<IOuterLoopCriterion> criterion,
                     std::shared_ptr<IMasterUpdate> master_updater,
                     pBendersBase benders)
    : criterion_(std::move(criterion)),
      master_updater_(std::move(master_updater)),
      benders_(std::move(benders)) {}

void OuterLoop::Run() {
  master_updater_->AddConstraints();
  master_updater_->AddCutsInMaster();

  bool criterion_is_ok = false;

  while (!criterion_is_ok) {
    benders_->launch();
    if (!criterion_->Check()) {
      master_updater_->Update();
    }
  }
}