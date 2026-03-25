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
    benders_->init_data(lambda, lambda_min, lambda_max);
}

void OuterLoopBendersAdapter::SaveOuterLoopSolutionInOutputFile() const
{
    benders_->SaveOuterLoopSolutionInOutputFile();
}

void OuterLoopBendersAdapter::SaveCurrentOuterLoopIterationInOutputFile() const
{
    benders_->SaveCurrentOuterLoopIterationInOutputFile();
}

void OuterLoopBendersAdapter::SetBilevelBestub(double bilevel_best_ub)
{
    benders_->SetBilevelBestub(bilevel_best_ub);
}

void OuterLoopBendersAdapter::UpdateOuterLoopSolution()
{
    benders_->UpdateOuterLoopSolution();
}

Output::SolutionData OuterLoopBendersAdapter::GetOuterLoopSolution() const
{
    return benders_->GetOuterLoopSolution();
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

} // namespace Outerloop

