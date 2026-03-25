/**
 * @file OuterLoopBendersDataAccessor.cpp
 * @brief Implementation of OuterLoopBendersDataAccessor
 */

#include "antares-xpansion/benders/outer_loop/OuterLoopBendersDataAccessor.h"
#include "antares-xpansion/benders/outer_loop/OuterLoopBendersAdapter.h"

namespace Outerloop
{

OuterLoopBendersDataAccessor::OuterLoopBendersDataAccessor(
  std::shared_ptr<OuterLoopBendersAdapter> adapter)
    : adapter_(std::move(adapter))
{
    if (!adapter_)
    {
        throw std::invalid_argument("OuterLoopBendersAdapter pointer cannot be null");
    }
}

Output::SolutionData OuterLoopBendersDataAccessor::GetBendersSolution() const
{
    return adapter_->GetOuterLoopSolution();
}

std::vector<double> OuterLoopBendersDataAccessor::GetOuterLoopCriteria() const
{
    return adapter_->GetOuterLoopCriterionAtBestBenders();
}

LambdaParameters OuterLoopBendersDataAccessor::GetLambdaParameters() const
{
    return LambdaParameters{
      adapter_->GetLambda(),
      adapter_->GetLambdaMin(),
      adapter_->GetLambdaMax()
    };
}

double OuterLoopBendersDataAccessor::GetBilevelBestub() const
{
    return adapter_->GetBilevelBestub();
}

void OuterLoopBendersDataAccessor::SetBilevelBestub(double value)
{
    adapter_->SetBilevelBestub(value);
}

int OuterLoopBendersDataAccessor::GetBendersRunNumber() const
{
    return adapter_->GetBendersRunNumber();
}

}  // namespace Outerloop

