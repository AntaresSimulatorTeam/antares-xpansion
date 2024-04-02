#include "OuterLoop.h"

#include "LoggerUtils.h"
using namespace Outerloop;

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

  benders_->ExternalLoopCheckFeasibility();

  bool stop_update_master = false;
  while (!stop_update_master) {
    PrintLog();
    benders_->init_data(master_updater_->Rhs());
    benders_->launch();
    benders_->RunExternalLoopBilevelChecks();
    if (world_.rank() == 0) {
      stop_update_master = master_updater_->Update(
          benders_->ExternalLoopLambdaMin(), benders_->ExternalLoopLambdaMax());
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
  // TODO criterion per pattern (aka prefix+area) and why at best Benders ?
  const auto outer_loop_criterion =
      benders_->GetOuterLoopCriterionAtBestBenders();
  auto sum_loss =
      outer_loop_criterion.size() == 0 ? 0 : outer_loop_criterion[0];
  msg << "*** Sum loss: " << std::scientific << std::setprecision(10)
      << sum_loss;
  logger->display_message(msg.str());
  logger->PrintIterationSeparatorEnd();
}
