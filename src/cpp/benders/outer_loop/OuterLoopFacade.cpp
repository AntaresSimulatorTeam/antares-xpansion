#include "antares-xpansion/benders/outer_loop/OuterLoopFacade.h"

OuterLoopFacade::OuterLoopFacade(pBendersBase benders): benders_(std::move(benders))
{
}

// Lifecycle

void OuterLoopFacade::Launch()
{
    benders_->launch();
}

void OuterLoopFacade::Free()
{
    benders_->free();
}

void OuterLoopFacade::InitializeProblems()
{
    benders_->InitializeProblems();
}

void OuterLoopFacade::DoFreeProblems(bool val)
{
    benders_->DoFreeProblems(val);
}

void OuterLoopFacade::InitData(double lambda, double lambda_min, double lambda_max)
{
    benders_->init_data(lambda, lambda_min, lambda_max);
}

// State

bool OuterLoopFacade::IsExceptionRaised() const
{
    return benders_->isExceptionRaised();
}

void OuterLoopFacade::IncrementRunNumber()
{
    benders_->IncrementBendersRunNumber();
}

int OuterLoopFacade::GetRunNumber() const
{
    return benders_->GetBendersRunNumber();
}

CurrentIterationData OuterLoopFacade::GetCurrentIterationData() const
{
    return benders_->GetCurrentIterationData();
}

WorkerMasterData OuterLoopFacade::BestIterationWorkerMaster() const
{
    return benders_->BestIterationWorkerMaster();
}

BendersBaseOptions OuterLoopFacade::GetOptions() const
{
    return benders_->Options();
}

void OuterLoopFacade::UpdateOverallCosts()
{
    benders_->UpdateOverallCosts();
}

// Logging

Logger OuterLoopFacade::GetLogger() const
{
    return benders_->GetOutputManager().GetLogger();
}

std::shared_ptr<MathLoggerDriver> OuterLoopFacade::GetMathLoggerDriver() const
{
    return benders_->GetOutputManager().GetMathLoggerDriver();
}

// Master operations

std::vector<double> OuterLoopFacade::GetMasterObjectiveFunctionCoeffs() const
{
    return benders_->GetMasterManager().GetObjectiveFunctionCoeffs();
}

void OuterLoopFacade::SetMasterObjectiveFunctionCoeffsToZeros()
{
    benders_->GetMasterManager().SetObjectiveFunctionCoeffsToZeros();
}

void OuterLoopFacade::SetMasterObjectiveFunction(const double* coeffs, int first, int last)
{
    benders_->GetMasterManager().SetObjectiveFunction(coeffs, first, last);
}

const VariableMap& OuterLoopFacade::GetMasterVariableMap() const
{
    return benders_->GetMasterManager().GetVariableMap();
}

// Outer loop operations

CriteriaCurrentIterationData OuterLoopFacade::GetOuterLoopData() const
{
    return benders_->GetOuterLoopManager().GetOuterLoopData();
}

std::vector<double> OuterLoopFacade::GetOuterLoopCriterionAtBestBenders() const
{
    return benders_->GetOuterLoopManager().GetOuterLoopCriterionAtBestBenders();
}

void OuterLoopFacade::UpdateOuterLoopSolution()
{
    benders_->GetOuterLoopManager().UpdateOuterLoopSolution();
}

void OuterLoopFacade::SaveCurrentOuterLoopIterationInOutputFile() const
{
    benders_->GetOuterLoopManager().SaveCurrentOuterLoopIterationInOutputFile();
}

void OuterLoopFacade::SetBilevelBestub(double val)
{
    benders_->GetOuterLoopManager().SetBilevelBestub(val);
}

void OuterLoopFacade::SaveOuterLoopSolutionInOutputFile() const
{
    benders_->GetOuterLoopManager().SaveOuterLoopSolutionInOutputFile();
}
