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
  benders_->launch();

  // by default LESSER?
  auto criterion = CRITERION::LESSER;

  while (criterion != CRITERION::EQUAL) {
    criterion =
        criterion_->IsCriterionSatisfied(benders_->CutsCurrentIteration());
    master_updater_->Update(criterion, benders_);
    benders_->launch();
  }
}