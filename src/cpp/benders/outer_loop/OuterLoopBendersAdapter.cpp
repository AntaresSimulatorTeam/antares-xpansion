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

    benders_->init_data();
}

void OuterLoopBendersAdapter::RefreshOuterLoopStateFromBenders()
{
    // Adapter-owned fields persist across the bulk copy from Benders.
    const auto preserved_lambda = current_outer_loop_data_.lambda;
    const auto preserved_lambda_min = current_outer_loop_data_.lambda_min;
    const auto preserved_lambda_max = current_outer_loop_data_.lambda_max;
    const auto preserved_bilevel_best_ub = current_outer_loop_data_.outer_loop_bilevel_best_ub;
    const auto preserved_benders_num_run = current_outer_loop_data_.benders_num_run;

    const auto current_data = benders_->GetCurrentIterationData();
    current_outer_loop_data_ = current_data.criteria_current_iteration_data;

    current_outer_loop_data_.lambda = preserved_lambda;
    current_outer_loop_data_.lambda_min = preserved_lambda_min;
    current_outer_loop_data_.lambda_max = preserved_lambda_max;
    current_outer_loop_data_.outer_loop_bilevel_best_ub = preserved_bilevel_best_ub;
    current_outer_loop_data_.benders_num_run = preserved_benders_num_run;

    const auto& criteria_per_it = benders_->GetCriteriaPerIteration();
    const auto best_it = current_data.best_it;
    outer_loop_criterion_at_best_benders_ =
        (criteria_per_it.empty() || best_it < 1) ? std::vector<double>()
                                                  : criteria_per_it[best_it - 1];
}

void OuterLoopBendersAdapter::SaveOuterLoopSolutionInOutputFile() const
{
    benders_->_writer->write_solution(outer_loop_solution_data_);
    benders_->_writer->dump();
}

void OuterLoopBendersAdapter::SaveCurrentOuterLoopIterationInOutputFile()
{
    UpdateCurrentOuterLoopIterationSnapshot();
    if (!current_outer_loop_iteration_.has_value())
    {
        return;
    }

    benders_->_writer->write_iteration(current_outer_loop_iteration_.value(),
                                       current_outer_loop_iteration_num_);
    benders_->_writer->dump();
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

double OuterLoopBendersAdapter::GetBilevelBestub() const
{
    return current_outer_loop_data_.outer_loop_bilevel_best_ub;
}

LambdaParameters OuterLoopBendersAdapter::GetLambdaParameters() const
{
    return {current_outer_loop_data_.lambda,
            current_outer_loop_data_.lambda_min,
            current_outer_loop_data_.lambda_max};
}

void OuterLoopBendersAdapter::UpdateCurrentOuterLoopIterationSnapshot()
{
    const auto last = benders_->GetLastWorkerMasterData();
    if (!last.has_value())
    {
        current_outer_loop_iteration_ = std::nullopt;
        current_outer_loop_iteration_num_ = 0;
        return;
    }
    const auto& d = last.value();
    Output::Iteration iter;
    iter.master_duration = d._master_duration;
    iter.subproblem_duration = d._subproblem_duration;
    iter.lb = d._lb;
    iter.ub = d._ub;
    iter.best_ub = d._best_ub;
    iter.optimality_gap = d._best_ub - d._lb;
    iter.relative_gap = (d._best_ub - d._lb) / d._best_ub;
    iter.investment_cost = d._invest_cost;
    iter.operational_cost = d._operational_cost;
    iter.overall_cost = d._invest_cost + d._operational_cost;
    iter.candidates = candidates_data(d);
    iter.cumulative_number_of_subproblem_resolved =
        benders_->GetTotalCumulativeSubproblemSolvedCount();
    current_outer_loop_iteration_ = iter;
    current_outer_loop_iteration_num_ = current_outer_loop_data_.benders_num_run;
}

Logger OuterLoopBendersAdapter::GetLogger() const
{
    return benders_->_logger;
}

std::shared_ptr<MathLoggerDriver> OuterLoopBendersAdapter::GetMathLoggerDriver() const
{
    return benders_->mathLoggerDriver_;
}

void OuterLoopBendersAdapter::DoFreeProblems(bool free_problems)
{
    benders_->DoFreeProblems(free_problems);
}

void OuterLoopBendersAdapter::InitializeProblems()
{
    benders_->InitializeProblems();
}

void OuterLoopBendersAdapter::Launch()
{
    benders_->launch();
    RefreshOuterLoopStateFromBenders();
}

void OuterLoopBendersAdapter::Free()
{
    benders_->free();
}

bool OuterLoopBendersAdapter::IsExceptionRaised() const
{
    return benders_->isExceptionRaised();
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
