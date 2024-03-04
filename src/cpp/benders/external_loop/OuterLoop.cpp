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
      world_(world) {
  loggers_.AddLogger(benders_->_logger);
  loggers_.AddLogger(benders_->mathLoggerDriver_);
}

void OuterLoop::Run() {
  benders_->DoFreeProblems(false);
  benders_->InitializeProblems();
  benders_->InitExternalValues();

  auto obj_coeff = benders_->MasterObjectiveFunctionCoeffs();

  // /!\ partially
  benders_->SetMasterObjectiveFunctionCoeffsToZeros();
  PrintLog();
  benders_->launch();

  benders_->SetMasterObjectiveFunction(obj_coeff.data(), 0,
                                       obj_coeff.size() - 1);

  // de-comment for general case
  //  cuts_manager_->Save(benders_->AllCuts());
  // auto cuts = cuts_manager_->Load();
  auto criterion =
      criterion_->IsCriterionSatisfied(benders_->BestIterationWorkerMaster());
  if (criterion == CRITERION::HIGH) {
    std::ostringstream err_msg;
    err_msg << PrefixMessage(LogUtils::LOGLEVEL::FATAL, "External Loop")
            << "Criterion cannot be satisfied for your study:\n"
            << criterion_->StateAsString();
    throw CriterionCouldNotBeSatisfied(err_msg.str(), LOGLOCATION);
  }

  // lambda_max
  master_updater_->Init();

  while (criterion != CRITERION::IS_MET) {
    benders_->ResetData(criterion_->CriterionValue());
    PrintLog();
    benders_->launch();
    criterion =
        criterion_->IsCriterionSatisfied(benders_->BestIterationWorkerMaster());
    master_updater_->Update(criterion);
  }
  // last prints
  PrintLog();
  auto benders_data = benders_->GetCurrentIterationData();
  benders_data.external_loop_criterion = criterion_->CriterionValue();
  benders_->mathLoggerDriver_->Print(benders_data);

  // TODO general-case
  //  cuts_manager_->Save(benders_->AllCuts());
  benders_->free();
}

void OuterLoop::PrintLog() {
  std::ostringstream msg;
  // just for operational log?
  auto logger = benders_->_logger;
  logger->PrintIterationSeparatorBegin();
  msg << "*** Outer loop: " << benders_->GetBendersRunNumber();
  logger->display_message(msg.str());
  msg.str("");
  msg << "*** Criterion value: " << std::scientific << std::setprecision(10)
      << criterion_->CriterionValue();
  logger->display_message(msg.str());
  logger->PrintIterationSeparatorEnd();
}