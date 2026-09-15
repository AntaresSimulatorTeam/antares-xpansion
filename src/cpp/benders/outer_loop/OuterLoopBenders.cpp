#include "antares-xpansion/benders/outer_loop/OuterLoopBenders.h"

namespace Outerloop
{

OuterLoopBenders::OuterLoopBenders(
  const std::vector<Benders::Criterion::CriterionSingleInputData>& outer_loop_data,
  std::shared_ptr<IMasterUpdate> master_updater,
  std::shared_ptr<ICutsManager> cuts_manager,
  std::shared_ptr<OuterLoopFacade> facade,
  std::shared_ptr<ICommunicationStrategy> communication_strategy):
    master_updater_(std::move(master_updater)),
    cuts_manager_(std::move(cuts_manager)),
    facade_(std::move(facade)),
    communication_strategy_(std::move(communication_strategy)),
    outer_loop_biLevel_(outer_loop_data)
{
    loggers_.AddLogger(facade_->GetLogger());
    loggers_.AddLogger(facade_->GetMathLoggerDriver());
    facade_->DoFreeProblems(false);
    facade_->InitializeProblems();
}

void OuterLoopBenders::PrintLog()
{
    std::ostringstream msg;
    auto logger = facade_->GetLogger();
    logger->PrintIterationSeparatorBegin();
    msg << "*** Adequacy criterion loop: " << facade_->GetRunNumber();
    logger->display_message(msg.str());
    msg.str("");
    const auto outer_loop_data = facade_->GetOuterLoopData();
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
    facade_->IncrementRunNumber();
    facade_->Launch();
}

void OuterLoopBenders::init_data()
{
    facade_->InitData(master_updater_->Rhs(), OuterLoopLambdaMin(), OuterLoopLambdaMax());
}

bool OuterLoopBenders::isExceptionRaised()
{
    return facade_->IsExceptionRaised();
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
    if (communication_strategy_->IsMaster())
    {
        stop_update_master = master_updater_->Update(outer_loop_biLevel_.LambdaMin(),
                                                     outer_loop_biLevel_.LambdaMax());
    }

    communication_strategy_->BroadcastBool(stop_update_master);
    return stop_update_master;
}

void OuterLoopBenders::OuterLoopCheckFeasibility()
{
    std::vector<double> obj_coeff;
    if (communication_strategy_->IsMaster())
    {
        obj_coeff = facade_->GetMasterObjectiveFunctionCoeffs();

        // /!\ partially
        facade_->SetMasterObjectiveFunctionCoeffsToZeros();
    }

    facade_->Launch();
    if (communication_strategy_->IsMaster())
    {
        facade_->SetMasterObjectiveFunction(obj_coeff.data(), 0, obj_coeff.size() - 1);
        facade_->UpdateOverallCosts();
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
    outer_loop_biLevel_.Init(facade_->GetMasterObjectiveFunctionCoeffs(),
                             facade_->BestIterationWorkerMaster().get_max_invest(),
                             facade_->GetMasterVariableMap());
    outer_loop_biLevel_.SetLambda(lambda);
}

void OuterLoopBenders::OuterLoopBilevelChecks()
{
    if (communication_strategy_->IsMaster()
        && (facade_->GetOptions().EXTERNAL_LOOP_OPTIONS.DO_OUTER_LOOP && !is_bilevel_check_all_))
    {
        const WorkerMasterData& workerMasterData = facade_->BestIterationWorkerMaster();
        const auto& invest_cost = workerMasterData._invest_cost;
        const auto& overall_cost = invest_cost + workerMasterData._operational_cost;
        const auto& x_cut = facade_->GetCurrentIterationData().solution.x_cut;
        const auto& external_loop_lambda = facade_->GetCurrentIterationData().criteria.lambda;
        if (outer_loop_biLevel_.Update_bilevel_data_if_feasible(
              x_cut,
              facade_->GetOuterLoopCriterionAtBestBenders() /*/!\ must
  be at best it*/
              ,
              overall_cost,
              invest_cost,
              external_loop_lambda))
        {
            facade_->UpdateOuterLoopSolution();
        }
        facade_->SaveCurrentOuterLoopIterationInOutputFile();
        facade_->SetBilevelBestub(outer_loop_biLevel_.BilevelBestub());
    }
}

void OuterLoopBenders::Run()
{
    OuterLoop::Run();
    facade_->GetMathLoggerDriver()->Print(facade_->GetCurrentIterationData());
    facade_->SaveOuterLoopSolutionInOutputFile();
    facade_->Free();
}
} // namespace Outerloop
