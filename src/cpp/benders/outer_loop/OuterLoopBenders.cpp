#include "antares-xpansion/benders/outer_loop/OuterLoopBenders.h"

#include <iomanip>
#include <sstream>

namespace Outerloop
{

OuterLoopBenders::OuterLoopBenders(
  const std::vector<Benders::Criterion::CriterionSingleInputData>& outer_loop_data,
  std::shared_ptr<IMasterUpdate> master_updater,
  pBendersBase benders,
  std::shared_ptr<ICommunicationStrategy> communication_strategy):
    master_updater_(std::move(master_updater)),
    adapter_(std::make_shared<OuterLoopBendersAdapter>(std::move(benders))),
    communication_strategy_(std::move(communication_strategy)),
    outer_loop_biLevel_(outer_loop_data)
{
    loggers_.AddLogger(adapter_->GetLogger());
    loggers_.AddLogger(adapter_->GetMathLoggerDriver());
    adapter_->DoFreeProblems(false);
    adapter_->InitializeProblems();
}

void OuterLoopBenders::PrintLog()
{
    std::ostringstream msg;
    auto logger = adapter_->GetLogger();
    logger->PrintIterationSeparatorBegin();
    msg << "*** Adequacy criterion loop: " << adapter_->GetBendersRunNumber();
    logger->display_message(msg.str());
    msg.str("");
    const auto outer_loop_data = adapter_->GetOuterLoopData();
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
    adapter_->IncrementBendersRunNumber();
    adapter_->Launch();
}

void OuterLoopBenders::init_data()
{
    adapter_->InitOuterLoopData(master_updater_->Rhs(), OuterLoopLambdaMin(), OuterLoopLambdaMax());
}

bool OuterLoopBenders::isExceptionRaised()
{
    return adapter_->IsExceptionRaised();
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
        obj_coeff = adapter_->MasterObjectiveFunctionCoeffs();

        // /!\ partially
        adapter_->SetMasterObjectiveFunctionCoeffsToZeros();
    }

    adapter_->Launch();
    if (communication_strategy_->IsMaster())
    {
        adapter_->SetMasterObjectiveFunction(obj_coeff.data(), 0, obj_coeff.size() - 1);
        adapter_->UpdateOverallCosts();
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
    outer_loop_biLevel_.Init(adapter_->MasterObjectiveFunctionCoeffs(),
                             adapter_->BestIterationWorkerMaster().get_max_invest(),
                             adapter_->MasterVariables());
    outer_loop_biLevel_.SetLambda(lambda);
}

void OuterLoopBenders::OuterLoopBilevelChecks()
{
    if (communication_strategy_->IsMaster() && (adapter_->DoOuterLoop() && !is_bilevel_check_all_))
    {
        const WorkerMasterData& workerMasterData = adapter_->BestIterationWorkerMaster();
        const auto& invest_cost = workerMasterData._invest_cost;
        const auto& overall_cost = invest_cost + workerMasterData._operational_cost;
        const auto current_iteration_data = adapter_->GetCurrentIterationData();
        const auto& x_cut = current_iteration_data.x_cut;
        const auto& external_loop_lambda = current_iteration_data.criteria_current_iteration_data
                                             .lambda;
        if (outer_loop_biLevel_.Update_bilevel_data_if_feasible(
              x_cut,
              adapter_->GetOuterLoopCriterionAtBestBenders() /*/!\ must
  be at best it*/
              ,
              overall_cost,
              invest_cost,
              external_loop_lambda))
        {
            adapter_->UpdateOuterLoopSolution();
        }
        adapter_->SaveCurrentOuterLoopIterationInOutputFile();
        adapter_->SetBilevelBestub(outer_loop_biLevel_.BilevelBestub());
    }
}

void OuterLoopBenders::Run()
{
    OuterLoop::Run();
    adapter_->GetMathLoggerDriver()->Print(adapter_->GetCurrentIterationData());
    adapter_->SaveOuterLoopSolutionInOutputFile();
    adapter_->Free();
}
} // namespace Outerloop
