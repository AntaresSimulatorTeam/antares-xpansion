#include "antares-xpansion/benders/benders_mpi/OuterLoopBenders.h"

namespace Outerloop
{

OuterLoopBenders::OuterLoopBenders(
  const std::vector<Benders::Criterion::CriterionSingleInputData>& outer_loop_data,
  std::shared_ptr<IMasterUpdate> master_updater,
  pBendersBase benders,
  mpi::communicator& world):
    master_updater_(std::move(master_updater)),
    benders_(std::move(benders)),
    adapter_(benders_),
    world_(world),
    outer_loop_biLevel_(outer_loop_data)
{
    loggers_.AddLogger(benders_->_logger);
    loggers_.AddLogger(benders_->mathLoggerDriver_);
    benders_->DoFreeProblems(false);
    benders_->InitializeProblems();
}

void OuterLoopBenders::PrintLog()
{
    std::ostringstream msg;
    auto logger = benders_->_logger;
    logger->PrintIterationSeparatorBegin();
    msg << "*** Adequacy criterion loop: " << adapter_.GetBendersRunNumber();
    logger->display_message(msg.str());
    msg.str("");
    const auto outer_loop_data = adapter_.GetOuterLoopData();
    msg << "*** Max Criterion: " << std::scientific << std::setprecision(10)
        << outer_loop_data.max_criterion_best_it;
    logger->display_message(msg.str());
    msg.str("");
    msg << "*** Max Criterion Area: " << outer_loop_data.max_criterion_area_best_it;
    logger->display_message(msg.str());
    logger->PrintIterationSeparatorEnd();
}

void OuterLoopBenders::RunAttachedAlgo()
{
    adapter_.IncrementBendersRunNumber();
    benders_->launch();
}

void OuterLoopBenders::init_data()
{
    adapter_.InitOuterLoopData(master_updater_->Rhs(), OuterLoopLambdaMin(), OuterLoopLambdaMax());
}

bool OuterLoopBenders::isExceptionRaised()
{
    return benders_->isExceptionRaised();
}

double OuterLoopBenders::OuterLoopLambdaMin() const
{
    return outer_loop_biLevel_.LambdaMin();
}

double OuterLoopBenders::OuterLoopLambdaMax() const
{
    return outer_loop_biLevel_.LambdaMax();
}

bool OuterLoopBenders::UpdateMaster()
{
    bool stop_update_master = false;
    if (world_.rank() == 0)
    {
        stop_update_master = master_updater_->Update(outer_loop_biLevel_.LambdaMin(),
                                                     outer_loop_biLevel_.LambdaMax());
    }

    mpi::broadcast(world_, stop_update_master, 0);
    return stop_update_master;
}

void OuterLoopBenders::OuterLoopCheckFeasibility()
{
    std::vector<double> obj_coeff;
    if (world_.rank() == 0)
    {
        obj_coeff = benders_->MasterObjectiveFunctionCoeffs();

        // /!\ partially
        benders_->SetMasterObjectiveFunctionCoeffsToZeros();
    }

    benders_->launch();
    if (world_.rank() == 0)
    {
        benders_->SetMasterObjectiveFunction(obj_coeff.data(), 0, obj_coeff.size() - 1);
        adapter_.UpdateOverallCosts();
        OuterLoopBilevelChecks();
        if (!outer_loop_biLevel_.FoundFeasible())
        {
            std::ostringstream err_msg;
            err_msg << PrefixMessage(LogUtils::LOGLEVEL::FATAL, "Outer Loop")
                    << "Criterion cannot be satisfied for your study\n";
            throw CriterionCouldNotBeSatisfied(err_msg.str(), LOGLOCATION);
        }

        InitExternalValues(false, 0.0);
    }
}

void OuterLoopBenders::InitExternalValues(bool is_bilevel_check_all, double lambda)
{
    is_bilevel_check_all_ = is_bilevel_check_all;
    outer_loop_biLevel_.Init(benders_->MasterObjectiveFunctionCoeffs(),
                             benders_->BestIterationWorkerMaster().get_max_invest(),
                             benders_->MasterVariables());
    outer_loop_biLevel_.SetLambda(lambda);
}

void OuterLoopBenders::OuterLoopBilevelChecks()
{
    if (world_.rank() == 0 && benders_->Options().EXTERNAL_LOOP_OPTIONS.DO_OUTER_LOOP
        && !is_bilevel_check_all_)
    {
        const WorkerMasterData& workerMasterData = benders_->BestIterationWorkerMaster();
        const auto& invest_cost = workerMasterData._invest_cost;
        const auto& overall_cost = invest_cost + workerMasterData._operational_cost;
        const auto& x_cut = benders_->GetCurrentIterationData().x_cut;
        const auto& external_loop_lambda = benders_->GetCurrentIterationData()
                                             .criteria_current_iteration_data.lambda;
        if (outer_loop_biLevel_.Update_bilevel_data_if_feasible(
              x_cut,
              adapter_.GetOuterLoopCriterionAtBestBenders() /*/!\ must
  be at best it*/
              ,
              overall_cost,
              invest_cost,
              external_loop_lambda))
        {
            adapter_.UpdateOuterLoopSolution();
        }
        adapter_.SaveCurrentOuterLoopIterationInOutputFile();
        adapter_.SetBilevelBestub(outer_loop_biLevel_.BilevelBestub());
    }
}

void OuterLoopBenders::Run()
{
    OuterLoop::Run();
    benders_->mathLoggerDriver_->Print(benders_->GetCurrentIterationData());
    adapter_.SaveOuterLoopSolutionInOutputFile();
    benders_->free();
}
} // namespace Outerloop
