#include "OuterLoop.h"

#include "LoggerUtils.h"
using namespace Outerloop;

OuterLoop::OuterLoop(std::shared_ptr<IMasterUpdate> master_updater,
                     std::shared_ptr<ICutsManager> cuts_manager,
                     pBendersBase benders, mpi::environment& env,
                     mpi::communicator& world)
    : master_updater_(std::move(master_updater)),
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
  Output::SolutionData solution_data = benders_->solution();
  solution_data.solution = benders_->iteration(benders_->_data.outer_loop_current_iteration_data.outer_loop_best_master_data);

  PrintLog();
  benders_->mathLoggerDriver_->Print(benders_->GetCurrentIterationData());
  benders_->_writer->write_solution();

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
