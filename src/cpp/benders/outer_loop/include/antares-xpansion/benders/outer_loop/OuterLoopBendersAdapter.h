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

    // === Phase B Migration ===
    /**
     * @brief Store the outer loop solution locally in adapter
     * (migrated from BendersBase::outer_loop_solution_data_)
     */
    void StoreOuterLoopSolutionLocally();

    /**
     * @brief Get bilevel best upper bound
     * (migrated from BendersBase::_data.criteria_current_iteration_data.outer_loop_bilevel_best_ub)
     */
    [[nodiscard]] double GetBilevelBestub() const;

    /**
     * @brief Get current lambda value for outer loop
     * (migrated from BendersBase::_data.criteria_current_iteration_data.lambda)
     */
    [[nodiscard]] double GetLambda() const;

    /**
     * @brief Get minimum lambda value
     * (migrated from BendersBase::_data.criteria_current_iteration_data.lambda_min)
     */
    [[nodiscard]] double GetLambdaMin() const;

    /**
     * @brief Get maximum lambda value
     * (migrated from BendersBase::_data.criteria_current_iteration_data.lambda_max)
     */
    [[nodiscard]] double GetLambdaMax() const;

private:
    pBendersBase benders_;

    // === Phase B: Locally stored outer loop data ===
    Output::SolutionData outer_loop_solution_data_;
    double bilevel_best_ub_ = 1e20;
    double lambda_ = 0.0;
    double lambda_min_ = 0.0;
    double lambda_max_ = 0.0;

    // Phase B Step 2: local snapshot for outer-loop iteration output
    mutable std::optional<Output::Iteration> current_outer_loop_iteration_;
    mutable int current_outer_loop_iteration_num_ = 0;

    void UpdateCurrentOuterLoopIterationSnapshot() const;
};

} // namespace Outerloop

