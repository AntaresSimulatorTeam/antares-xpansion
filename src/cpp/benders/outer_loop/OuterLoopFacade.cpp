#include "antares-xpansion/benders/outer_loop/OuterLoopFacade.h"

OuterLoopFacade::OuterLoopFacade(pBendersBase benders):
    benders_(std::move(benders)),
    master_manager_(benders_->GetMasterManager()),
    outer_loop_manager_(benders_->GetOuterLoopManager())
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
    return benders_->GetOutputManager()->GetLogger();
}

std::shared_ptr<MathLoggerDriver> OuterLoopFacade::GetMathLoggerDriver() const
{
    return benders_->GetOutputManager()->GetMathLoggerDriver();
}

// Master operations

std::vector<double> OuterLoopFacade::GetMasterObjectiveFunctionCoeffs() const
{
    return master_manager_->GetObjectiveFunctionCoeffs();
}

void OuterLoopFacade::SetMasterObjectiveFunctionCoeffsToZeros()
{
    master_manager_->SetObjectiveFunctionCoeffsToZeros();
}

void OuterLoopFacade::SetMasterObjectiveFunction(const double* coeffs, int first, int last)
{
    master_manager_->SetObjectiveFunction(coeffs, first, last);
}

const VariableMap& OuterLoopFacade::GetMasterVariableMap() const
{
    return master_manager_->GetVariableMap();
}

// Outer loop operations

CriteriaCurrentIterationData OuterLoopFacade::GetOuterLoopData() const
{
    return outer_loop_manager_->GetOuterLoopData();
}

std::vector<double> OuterLoopFacade::GetOuterLoopCriterionAtBestBenders() const
{
    return outer_loop_manager_->GetOuterLoopCriterionAtBestBenders();
}

void OuterLoopFacade::UpdateOuterLoopSolution()
{
    outer_loop_manager_->UpdateOuterLoopSolution();
}

void OuterLoopFacade::SaveCurrentOuterLoopIterationInOutputFile() const
{
    outer_loop_manager_->SaveCurrentOuterLoopIterationInOutputFile();
}

void OuterLoopFacade::SetBilevelBestub(double val)
{
    outer_loop_manager_->SetBilevelBestub(val);
}

void OuterLoopFacade::SaveOuterLoopSolutionInOutputFile() const
{
    outer_loop_manager_->SaveOuterLoopSolutionInOutputFile();
}
