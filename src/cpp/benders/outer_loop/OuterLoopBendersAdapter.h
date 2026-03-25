/**
 * @file OuterLoopBendersAdapter.h
 * @brief Phase B adapter: Progressively migrating outer loop logic from BendersBase
 *
 * This adapter encapsulates outer loop state that will be gradually migrated out of BendersBase.
 * It stores:
 * - outer_loop_solution_data_ (previously in BendersBase)
 * - bilevel_best_ub (previously in BendersBase::_data)
 * - Other outer loop specific data
 *
 * This is a temporary state during Phase B migration. Eventually all outer loop logic
 * will be in the outer loop classes.
 */

#pragma once

#include <memory>
#include <antares-xpansion/output/SolutionData.h>

class BendersBase;

namespace Outerloop
{
/**
 * @class OuterLoopBendersAdapter
 * @brief Adapter managing outer loop data migration from BendersBase
 *
 * Phase B: Progressive migration
 * - Stores outer loop specific data previously in BendersBase
 * - Provides clean interface for outer loop operations
 * - Preserves BendersBase interface during transition
 */
class OuterLoopBendersAdapter
{
public:
    /**
     * @brief Constructor
     * @param benders Shared pointer to BendersBase instance
     */
    explicit OuterLoopBendersAdapter(std::shared_ptr<BendersBase> benders);

    /**
     * @brief Update the stored outer loop solution from current Benders solution
     */
    void UpdateOuterLoopSolution();

    /**
     * @brief Get the stored outer loop solution
     * @return The current outer loop solution data
     */
    [[nodiscard]] Output::SolutionData GetOuterLoopSolution() const;

    /**
     * @brief Save outer loop solution to output file
     */
    void SaveOuterLoopSolutionInOutputFile() const;

    /**
     * @brief Set the bilevel best upper bound
     * @param bilevel_best_ub The value to set
     */
    void SetBilevelBestub(double bilevel_best_ub);

    /**
     * @brief Get the bilevel best upper bound
     * @return The current bilevel best upper bound
     */
    [[nodiscard]] double GetBilevelBestub() const;

    /**
     * @brief Initialize outer loop data with lambda parameters
     * @param external_loop_lambda Current lambda value
     * @param external_loop_lambda_min Minimum lambda value
     * @param external_loop_lambda_max Maximum lambda value
     */
    void InitializeOuterLoopData(double external_loop_lambda,
                                 double external_loop_lambda_min,
                                 double external_loop_lambda_max);

    /**
     * @brief Check if outer loop should run
     * @return true if outer loop is enabled in options
     */
    [[nodiscard]] bool ShouldRunOuterLoop() const;

private:
    std::shared_ptr<BendersBase> benders_;  ///< Reference to BendersBase
    Output::SolutionData outer_loop_solution_data_;  ///< Migrated from BendersBase
    double bilevel_best_ub_ = 1e20;  ///< Migrated from BendersBase::_data
};

}  // namespace Outerloop

