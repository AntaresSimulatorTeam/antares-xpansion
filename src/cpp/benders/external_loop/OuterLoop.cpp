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
  bool criterion_check = false;
  std::vector<double> obj_coeff;
  if (world_.rank() == 0) {
    obj_coeff = benders_->MasterObjectiveFunctionCoeffs();

    // /!\ partially
    benders_->SetMasterObjectiveFunctionCoeffsToZeros();

    PrintLog();
  }
  benders_->launch();
  if (world_.rank() == 0) {
    benders_->SetMasterObjectiveFunction(obj_coeff.data(), 0,
                                         obj_coeff.size() - 1);
    // de-comment for general case
    //  cuts_manager_->Save(benders_->AllCuts());
    // auto cuts = cuts_manager_->Load();
    criterion_check =
        criterion_->IsCriterionHigh(benders_->GetOuterLoopCriterion());
    // High
    if (criterion_check) {
      std::ostringstream err_msg;
      err_msg << PrefixMessage(LogUtils::LOGLEVEL::FATAL, "External Loop")
              << "Criterion cannot be satisfied for your study:\n"
              << criterion_->StateAsString();
      throw CriterionCouldNotBeSatisfied(err_msg.str(), LOGLOCATION);
    }
    // lambda_max
    master_updater_->Init();
  }

  bool stop_update_master = false;
  while (!stop_update_master) {
    benders_->init_data();
    PrintLog();
    benders_->launch();
    if (world_.rank() == 0) {
      criterion_check =
          criterion_->IsCriterionHigh(benders_->GetOuterLoopCriterion());
      stop_update_master = master_updater_->Update(criterion_check);
    }

    mpi::broadcast(world_, stop_update_master, 0);
  }
  // last prints
  PrintLog();
  benders_->mathLoggerDriver_->Print(benders_->GetCurrentIterationData());

  // TODO general-case
  //  cuts_manager_->Save(benders_->AllCuts());
  benders_->free();
}

void OuterLoop::PrintLog() {
  std::ostringstream msg;
  auto logger = benders_->_logger;
  logger->PrintIterationSeparatorBegin();
  msg << "*** Outer loop: " << benders_->GetBendersRunNumber();
  logger->display_message(msg.str());
  msg.str("");
  msg << "*** Sum loss: " << std::scientific << std::setprecision(10)
      << criterion_->SumCriterions();
  logger->display_message(msg.str());
  logger->PrintIterationSeparatorEnd();
}