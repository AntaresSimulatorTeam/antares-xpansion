#include "antares-xpansion/benders/outer_loop/OuterLoopBendersAdapter.h"

namespace Outerloop
{

OuterLoopBendersAdapter::OuterLoopBendersAdapter(pBendersBase benders):
    benders_(std::move(benders))
{
}

CriteriaCurrentIterationData OuterLoopBendersAdapter::GetOuterLoopData() const
{
    return current_outer_loop_data_;
}

std::vector<double> OuterLoopBendersAdapter::GetOuterLoopCriterionAtBestBenders() const
{
    return outer_loop_criterion_at_best_benders_;
}

void OuterLoopBendersAdapter::InitOuterLoopData(double lambda,
                                                double lambda_min,
                                                double lambda_max)
{
    lambda_ = lambda;
    lambda_min_ = lambda_min;
    lambda_max_ = lambda_max;
    current_outer_loop_data_.lambda = lambda;
    current_outer_loop_data_.lambda_min = lambda_min;
    current_outer_loop_data_.lambda_max = lambda_max;
    current_outer_loop_data_.outer_loop_bilevel_best_ub = bilevel_best_ub_;

    benders_->init_data(lambda, lambda_min, lambda_max);
}

void OuterLoopBendersAdapter::RefreshOuterLoopStateFromBenders()
{
    current_outer_loop_data_ = benders_->GetOuterLoopData();
    outer_loop_criterion_at_best_benders_ = benders_->GetOuterLoopCriterionAtBestBenders();
    lambda_ = current_outer_loop_data_.lambda;
    lambda_min_ = current_outer_loop_data_.lambda_min;
    lambda_max_ = current_outer_loop_data_.lambda_max;
    bilevel_best_ub_ = current_outer_loop_data_.outer_loop_bilevel_best_ub;
}

void OuterLoopBendersAdapter::SaveOuterLoopSolutionInOutputFile() const
{
    benders_->_writer->write_solution(outer_loop_solution_data_);
    benders_->_writer->dump();
}

void OuterLoopBendersAdapter::SaveCurrentOuterLoopIterationInOutputFile() const
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
    bilevel_best_ub_ = bilevel_best_ub;
    current_outer_loop_data_.outer_loop_bilevel_best_ub = bilevel_best_ub;
    benders_->SetBilevelBestub(bilevel_best_ub);
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
    benders_->IncrementBendersRunNumber();
    ++current_outer_loop_data_.benders_num_run;
}

void OuterLoopBendersAdapter::StoreOuterLoopSolutionLocally()
{
    outer_loop_solution_data_ = benders_->GetCurrentBendersSolution();
    outer_loop_solution_data_.best_it = current_outer_loop_data_.benders_num_run;
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
    current_outer_loop_iteration_num_ = current_outer_loop_data_.benders_num_run;
}

} // namespace Outerloop

