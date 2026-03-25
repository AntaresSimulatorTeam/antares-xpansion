#pragma once

#include "antares-xpansion/benders/benders_core/BendersBase.h"

namespace Outerloop
{

/**
 * @brief Adapter that encapsulates outer-loop-specific operations on BendersBase.
 *
 * This class acts as a facade isolating the OuterLoop orchestration from
 * BendersBase's internal outer-loop state.  In Phase A every method simply
 * delegates to the corresponding BendersBase method; in later phases the
 * actual logic will migrate here and be removed from BendersBase.
 */
class OuterLoopBendersAdapter
{
public:
    explicit OuterLoopBendersAdapter(pBendersBase benders);

    CriteriaCurrentIterationData GetOuterLoopData() const;
    std::vector<double> GetOuterLoopCriterionAtBestBenders() const;

    void InitOuterLoopData(double lambda, double lambda_min, double lambda_max);

    void SaveOuterLoopSolutionInOutputFile() const;
    void SaveCurrentOuterLoopIterationInOutputFile() const;

    void SetBilevelBestub(double bilevel_best_ub);
    void UpdateOuterLoopSolution();
    Output::SolutionData GetOuterLoopSolution() const;

    void UpdateOverallCosts();

    int GetBendersRunNumber() const;
    void IncrementBendersRunNumber();

private:
    pBendersBase benders_;
};

} // namespace Outerloop

