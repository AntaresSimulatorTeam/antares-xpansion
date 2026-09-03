#include "antares-xpansion/benders/outer_loop/OuterLoopBendersAdapter.h"

namespace Outerloop
{

OuterLoopBendersAdapter::OuterLoopBendersAdapter(pBendersBase benders):
    benders_(std::move(benders))
{
    benders_->SuppressOutputFileWrites(true);
}

CriteriaCurrentIterationData OuterLoopBendersAdapter::GetOuterLoopData() const
{
    return current_outer_loop_data_;
}

std::vector<double> OuterLoopBendersAdapter::GetOuterLoopCriterionAtBestBenders() const
{
    return outer_loop_criterion_at_best_benders_;
}

void OuterLoopBendersAdapter::InitOuterLoopData(double lambda, double lambda_min, double lambda_max)
{
    current_outer_loop_data_.lambda = lambda;
    current_outer_loop_data_.lambda_min = lambda_min;
    current_outer_loop_data_.lambda_max = lambda_max;
}

void OuterLoopBendersAdapter::Launch()
{
    benders_->StartOuterLoopIteration(current_outer_loop_data_);
    benders_->launch();
    RefreshOuterLoopStateFromBenders();
}

void OuterLoopBendersAdapter::RefreshOuterLoopStateFromBenders()
{
    // Adapter-owned fields are preserved; criteria fields are refreshed from
    // the engine.
    const auto preserved = current_outer_loop_data_;
    const auto current_data = benders_->GetCurrentIterationData();
    current_outer_loop_data_ = current_data.criteria_current_iteration_data;
    current_outer_loop_data_.lambda = preserved.lambda;
    current_outer_loop_data_.lambda_min = preserved.lambda_min;
    current_outer_loop_data_.lambda_max = preserved.lambda_max;
    current_outer_loop_data_.benders_num_run = preserved.benders_num_run;
    current_outer_loop_data_.outer_loop_bilevel_best_ub = preserved.outer_loop_bilevel_best_ub;

    const auto& criteria_per_iteration = benders_->GetCriteriaPerIteration();
    const auto best_it = current_data.best_it;
    outer_loop_criterion_at_best_benders_ = (criteria_per_iteration.empty() || best_it < 1)
                                              ? std::vector<double>{}
                                              : criteria_per_iteration[static_cast<size_t>(best_it
                                                                                           - 1)];
}

void OuterLoopBendersAdapter::SaveOuterLoopSolutionInOutputFile() const
{
    benders_->_writer->write_solution(outer_loop_solution_data_);
    benders_->_writer->dump();
}

void OuterLoopBendersAdapter::SaveCurrentOuterLoopIterationInOutputFile()
{
    if (const auto iter = benders_->LastIterationSnapshot())
    {
        benders_->_writer->write_iteration(*iter, GetBendersRunNumber());
        benders_->_writer->dump();
    }
}

void OuterLoopBendersAdapter::SetBilevelBestub(double bilevel_best_ub)
{
    current_outer_loop_data_.outer_loop_bilevel_best_ub = bilevel_best_ub;
}

void OuterLoopBendersAdapter::UpdateOuterLoopSolution()
{
    outer_loop_solution_data_ = benders_->GetCurrentBendersSolution();
    outer_loop_solution_data_.best_it = current_outer_loop_data_.benders_num_run;
}

Output::SolutionData OuterLoopBendersAdapter::GetOuterLoopSolution() const
{
    return outer_loop_solution_data_;
}

void OuterLoopBendersAdapter::UpdateOverallCosts()
{
    benders_->UpdateOverallCosts();
}

int OuterLoopBendersAdapter::GetBendersRunNumber() const
{
    return current_outer_loop_data_.benders_num_run;
}

void OuterLoopBendersAdapter::IncrementBendersRunNumber()
{
    ++current_outer_loop_data_.benders_num_run;
}

void OuterLoopBendersAdapter::DoFreeProblems(bool free_problems)
{
    benders_->DoFreeProblems(free_problems);
}

void OuterLoopBendersAdapter::InitializeProblems()
{
    benders_->InitializeProblems();
}

void OuterLoopBendersAdapter::Free()
{
    benders_->free();
}

bool OuterLoopBendersAdapter::IsExceptionRaised() const
{
    return benders_->isExceptionRaised();
}

Logger OuterLoopBendersAdapter::GetLogger() const
{
    return benders_->_logger;
}

std::shared_ptr<MathLoggerDriver> OuterLoopBendersAdapter::GetMathLoggerDriver() const
{
    return benders_->mathLoggerDriver_;
}

std::vector<double> OuterLoopBendersAdapter::MasterObjectiveFunctionCoeffs() const
{
    return benders_->MasterObjectiveFunctionCoeffs();
}

void OuterLoopBendersAdapter::SetMasterObjectiveFunctionCoeffsToZeros() const
{
    benders_->SetMasterObjectiveFunctionCoeffsToZeros();
}

void OuterLoopBendersAdapter::SetMasterObjectiveFunction(const double* coeffs,
                                                         int first,
                                                         int last) const
{
    benders_->SetMasterObjectiveFunction(coeffs, first, last);
}

const VariableMap& OuterLoopBendersAdapter::MasterVariables() const
{
    return benders_->MasterVariables();
}

WorkerMasterData OuterLoopBendersAdapter::BestIterationWorkerMaster() const
{
    return benders_->BestIterationWorkerMaster();
}

CurrentIterationData OuterLoopBendersAdapter::GetCurrentIterationData() const
{
    auto data = benders_->GetCurrentIterationData();
    data.criteria_current_iteration_data = current_outer_loop_data_;
    return data;
}

bool OuterLoopBendersAdapter::DoOuterLoop() const
{
    return benders_->Options().EXTERNAL_LOOP_OPTIONS.DO_OUTER_LOOP;
}

} // namespace Outerloop
