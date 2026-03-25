#include "antares-xpansion/benders/outer_loop/OuterLoopBendersAdapter.h"

namespace Outerloop
{

OuterLoopBendersAdapter::OuterLoopBendersAdapter(pBendersBase benders):
    benders_(std::move(benders))
{
}

CriteriaCurrentIterationData OuterLoopBendersAdapter::GetOuterLoopData() const
{
    return benders_->GetOuterLoopData();
}

std::vector<double> OuterLoopBendersAdapter::GetOuterLoopCriterionAtBestBenders() const
{
    return benders_->GetOuterLoopCriterionAtBestBenders();
}

void OuterLoopBendersAdapter::InitOuterLoopData(double lambda,
                                                 double lambda_min,
                                                 double lambda_max)
{
    // Phase B: Store lambda locally in adapter
    lambda_ = lambda;
    lambda_min_ = lambda_min;
    lambda_max_ = lambda_max;

    benders_->init_data(lambda, lambda_min, lambda_max);
}


void OuterLoopBendersAdapter::SaveOuterLoopSolutionInOutputFile() const
{
    // Phase B: Use locally stored solution
    benders_->_writer->write_solution(outer_loop_solution_data_);
    benders_->_writer->dump();
}

void OuterLoopBendersAdapter::SaveCurrentOuterLoopIterationInOutputFile() const
{
    // Refresh local snapshot just before writing to keep number/data aligned.
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
    bilevel_best_ub_ = bilevel_best_ub;  // Phase B: Store locally
    benders_->SetBilevelBestub(bilevel_best_ub);  // Keep backward compat
}


void OuterLoopBendersAdapter::UpdateOuterLoopSolution()
{
    // Phase B: Store locally in adapter
    outer_loop_solution_data_ = benders_->GetCurrentBendersSolution();
    auto outer_loop_data = benders_->GetOuterLoopData();
    outer_loop_solution_data_.best_it = outer_loop_data.benders_num_run;

    // Keep backward compat with BendersBase (for now)
    benders_->UpdateOuterLoopSolution();
}

Output::SolutionData OuterLoopBendersAdapter::GetOuterLoopSolution() const
{
    // Phase B: Return locally stored solution
    return outer_loop_solution_data_;
}

void OuterLoopBendersAdapter::UpdateOverallCosts()
{
    benders_->UpdateOverallCosts();
}

int OuterLoopBendersAdapter::GetBendersRunNumber() const
{
    return benders_->GetBendersRunNumber();
}

void OuterLoopBendersAdapter::IncrementBendersRunNumber()
{
    benders_->IncrementBendersRunNumber();
}

// === Phase B Implementation ===

void OuterLoopBendersAdapter::StoreOuterLoopSolutionLocally()
{
    outer_loop_solution_data_ = benders_->GetCurrentBendersSolution();
    auto outer_loop_data = benders_->GetOuterLoopData();
    outer_loop_solution_data_.best_it = outer_loop_data.benders_num_run;
}

double OuterLoopBendersAdapter::GetBilevelBestub() const
{
    return bilevel_best_ub_;
}

double OuterLoopBendersAdapter::GetLambda() const
{
    return lambda_;
}

double OuterLoopBendersAdapter::GetLambdaMin() const
{
    return lambda_min_;
}

double OuterLoopBendersAdapter::GetLambdaMax() const
{
    return lambda_max_;
}

void OuterLoopBendersAdapter::UpdateCurrentOuterLoopIterationSnapshot() const
{
    current_outer_loop_iteration_ = benders_->GetLastOuterLoopIteration();
    current_outer_loop_iteration_num_ = benders_->GetBendersRunNumber();
}

} // namespace Outerloop
