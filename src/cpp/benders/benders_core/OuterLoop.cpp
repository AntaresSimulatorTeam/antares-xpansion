#include "OuterLoop.h"

OuterLoop::OuterLoop(std::shared_ptr<IOuterLoopCriterion> criterion,
                     std::shared_ptr<IMasterUpdate> master_updater,
                     std::shared_ptr<ICutsManager> cuts_manager,
                     pBendersBase benders)
    : criterion_(std::move(criterion)),
      master_updater_(std::move(master_updater)),
      cuts_manager_(std::move(cuts_manager)),
      benders_(std::move(benders)) {}

void OuterLoop::Run() {
  // addconstraint
  //  master_updater_->Update();
  // master_updater_->AddCutsInMaster();
  // benders_->launch();
  // cuts_manager_->Save(benders_->CutsCurrentIteration());
  // by default LESSER?
  CRITERION criterion = CRITERION::GREATER;

  // auto cuts = cuts_manager_->Load();
  // auto criterion = criterion_->IsCriterionSatisfied(cuts);
  while (criterion != CRITERION::EQUAL) {
    benders_->launch();
    cuts_manager_->Save(benders_->CutsCurrentIteration());
    auto cuts = cuts_manager_->Load();
    criterion = criterion_->IsCriterionSatisfied(cuts);
    master_updater_->Update(criterion);
  }
}