#include "OuterLoop.h"

#include "LoggerUtils.h"
OuterLoop::OuterLoop(std::shared_ptr<IOuterLoopCriterion> criterion,
                     std::shared_ptr<IMasterUpdate> master_updater,
                     std::shared_ptr<ICutsManager> cuts_manager,
                     pBendersBase benders, mpi::environment& env,
                     mpi::communicator& world)
    : criterion_(std::move(criterion)),
      master_updater_(std::move(master_updater)),
      cuts_manager_(std::move(cuts_manager)),
      benders_(std::move(benders)),
      env_(env),
      world_(world) {}

void OuterLoop::Run() {
  // by default GREATER?
  // CRITERION criterion = CRITERION::GREATER;
  benders_->DoFreeProblems(false);
  // invest cost
  benders_->InitializeProblems();

  auto obj_coeff = benders_->ObjectiveFunctionCoeffs();
  benders_->SetObjectiveFunctionCoeffsToZeros();
  // auto max_ite = benders_->Options().MAX_ITERATIONS;
  // benders_->SetMaxIteration(1);
  benders_->launch();

  benders_->SetObjectiveFunction(obj_coeff.data(), 0, obj_coeff.size() - 1);
  // benders_->SetMaxIteration(max_ite);

  // de-comment for general case
  //  cuts_manager_->Save(benders_->AllCuts());
  // auto cuts = cuts_manager_->Load();
  auto criterion =
      criterion_->IsCriterionSatisfied(benders_->BestIterationWorkerMaster());
  if (criterion == CRITERION::GREATER) {
    std::ostringstream err_msg;
    err_msg << PrefixMessage(LogUtils::LOGLEVEL::FATAL, "External Loop")
            << "Criterion cannot be satisfied for your study:\n"
            << criterion_->StateAsString();
    throw CriterionCouldNotBeSatisfied(err_msg.str(), LOGLOCATION);
  }
  // remove this line and past it before benders re-run
  benders_->ResetData();

  while (criterion != CRITERION::EQUAL) {
    // benders_->ResetData();
    benders_->launch();

    // de-comment for general case
    //  cuts_manager_->Save(benders_->AllCuts());
    //  auto cuts = cuts_manager_->Load();
    //  criterion = criterion_->IsCriterionSatisfied(cuts);
    criterion =
        criterion_->IsCriterionSatisfied(benders_->BestIterationWorkerMaster());
    master_updater_->Update(criterion);
  }
  cuts_manager_->Save(benders_->AllCuts());
  benders_->free();
}