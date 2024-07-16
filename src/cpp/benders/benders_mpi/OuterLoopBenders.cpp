#include "OuterLoopBenders.h"
namespace Outerloop {
OuterLoopBenders::OuterLoopBenders(
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

void OuterLoopBenders::Run() {
  benders_->DoFreeProblems(false);
  benders_->InitializeProblems();

  benders_->OuterLoopCheckFeasibility();

  bool stop_update_master = false;
  while (!stop_update_master) {
    PrintLog();
    benders_->init_data(master_updater_->Rhs(), benders_->OuterLoopLambdaMin(),
                        benders_->OuterLoopLambdaMax());
    benders_->launch();
    if (!benders_->isExceptionRaised()) {
      benders_->OuterLoopBilevelChecks();
      if (world_.rank() == 0) {
        stop_update_master = master_updater_->Update(
            benders_->OuterLoopLambdaMin(), benders_->OuterLoopLambdaMax());
      }

      mpi::broadcast(world_, stop_update_master, 0);
    } else {
      stop_update_master = true;
    }
  }

  // last prints
  PrintLog();
  benders_->mathLoggerDriver_->Print(benders_->GetCurrentIterationData());
  benders_->SaveOuterLoopSolutionInOutputFile();
  // TODO general-case
  //  cuts_manager_->Save(benders_->AllCuts());
  benders_->free();
}

void OuterLoopBenders::PrintLog() {
  std::ostringstream msg;
  auto logger = benders_->_logger;
  logger->PrintIterationSeparatorBegin();
  msg << "*** Adequacy criterion loop: " << benders_->GetBendersRunNumber();
  logger->display_message(msg.str());
  msg.str("");
  // TODO criterion per pattern (aka prefix+area) at best Benders ?
  const auto outer_loop_data = benders_->GetOuterLoopData();
  msg << "*** Max Criterion: " << std::scientific << std::setprecision(10)
      << outer_loop_data.max_criterion_best_it;
  logger->display_message(msg.str());
  msg.str("");
  msg << "*** Max Criterion Area: "
      << outer_loop_data.max_criterion_area_best_it;
  logger->display_message(msg.str());
  logger->PrintIterationSeparatorEnd();
}

void OuterLoopBenders::OuterLoopCheckFeasibility() {
  benders_->OuterLoopCheckFeasibility();
}

void OuterLoopBenders::OuterLoopBilevelChecks() {
  benders_->OuterLoopBilevelChecks();
}
}  // namespace Outerloop