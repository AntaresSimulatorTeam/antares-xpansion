#include "AdequacyCriterion.h"
#include "LoggerUtils.h"
using namespace AdequacyCriterionSpace;

AdequacyCriterion::AdequacyCriterion(
    std::shared_ptr<IMasterUpdate> master_updater,
    std::shared_ptr<ICutsManager> cuts_manager, pBendersBase benders,
    mpi::environment& env, mpi::communicator& world)
    : master_updater_(std::move(master_updater)),
      cuts_manager_(std::move(cuts_manager)),
      benders_(std::move(benders)),
      env_(env),
      world_(world) {
  loggers_.AddLogger(benders_->_logger);
  loggers_.AddLogger(benders_->mathLoggerDriver_);
}

void AdequacyCriterion::Run() {
  benders_->DoFreeProblems(false);
  benders_->InitializeProblems();

  benders_->ExternalLoopCheckFeasibility();

  bool stop_update_master = false;
  while (!stop_update_master) {
    PrintLog();
    benders_->init_data(master_updater_->Rhs(),
                        benders_->ExternalLoopLambdaMin(),
                        benders_->ExternalLoopLambdaMax());
    benders_->launch();
    if(!benders_->isExceptionRaised()) {
      benders_->RunExternalLoopBilevelChecks();
      if (world_.rank() == 0) {
        stop_update_master =
            master_updater_->Update(benders_->ExternalLoopLambdaMin(),
                                    benders_->ExternalLoopLambdaMax());
      }

      mpi::broadcast(world_, stop_update_master, 0);
    }
    else {
      stop_update_master = true;
    }
  }

  // last prints
  PrintLog();
  benders_->mathLoggerDriver_->Print(benders_->GetCurrentIterationData());
  benders_->SaveAdequacyCriterionSolutionInOutputFile();
  // TODO general-case
  //  cuts_manager_->Save(benders_->AllCuts());
  benders_->free();
}

void AdequacyCriterion::PrintLog() {
  std::ostringstream msg;
  auto logger = benders_->_logger;
  logger->PrintIterationSeparatorBegin();
  msg << "*** Adequacy Criterion: " << benders_->GetBendersRunNumber();
  logger->display_message(msg.str());
  msg.str("");
  // TODO criterion per pattern (aka prefix+area) at best Benders ?
  const auto adequacy_criterion_data = benders_->GetAdequacyCriterionData();
  msg << "*** Max Criterion: " << std::scientific << std::setprecision(10)
      << adequacy_criterion_data.max_criterion_best_it;
  logger->display_message(msg.str());
  msg.str("");
  msg << "*** Max Criterion Area: "
      << adequacy_criterion_data.max_criterion_area_best_it;
  logger->display_message(msg.str());
  logger->PrintIterationSeparatorEnd();
}
