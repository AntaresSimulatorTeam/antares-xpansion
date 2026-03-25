/**
 * @file OuterLoopBendersDataAccessor.h
 * @brief Clean interface for outer loop classes to access Benders data
 *
 * Phase B Step 4: Create a clean accessor interface
 *
 * This class provides a single point of access for outer loop components
 * to get Benders data. It shields them from implementation details of
 * both BendersBase and OuterLoopBendersAdapter.
 */

#pragma once

#include <memory>
#include <vector>
#include "antares-xpansion/benders/output/OutputWriter.h"

class BendersBase;

namespace Outerloop
{
class OuterLoopBendersAdapter;

/**
 * @struct LambdaParameters
 * @brief Container for lambda parameters
 */
struct LambdaParameters
{
    double lambda = 0.0;
    double lambda_min = 0.0;
    double lambda_max = 0.0;
};

/**
 * @class OuterLoopBendersDataAccessor
 * @brief Clean accessor interface for outer loop data access
 *
 * Provides isolated access to Benders data needed by outer loop classes.
 * This is a facade that shields outer loop logic from BendersBase internals.
 */
class OuterLoopBendersDataAccessor
{
public:
    /**
     * @brief Constructor
     * @param adapter Reference to adapter (owns Benders reference)
     */
    explicit OuterLoopBendersDataAccessor(
      std::shared_ptr<OuterLoopBendersAdapter> adapter);

    /**
     * @brief Get the current solution from Benders
     * @return The best solution found so far
     */
    [[nodiscard]] Output::SolutionData GetBendersSolution() const;

    /**
     * @brief Get all outer loop criteria at best Benders iteration
     * @return Vector of criterion values
     */
    [[nodiscard]] std::vector<double> GetOuterLoopCriteria() const;

    /**
     * @brief Get current lambda parameters for outer loop
     * @return LambdaParameters struct with current, min, max
     */
    [[nodiscard]] LambdaParameters GetLambdaParameters() const;

    /**
     * @brief Get the bilevel best upper bound
     * @return Current bilevel best UB value
     */
    [[nodiscard]] double GetBilevelBestub() const;

    /**
     * @brief Update the bilevel best upper bound
     * @param value The new value
     */
    void SetBilevelBestub(double value);

    /**
     * @brief Get current Benders run number
     * @return The iteration count within outer loop
     */
    [[nodiscard]] int GetBendersRunNumber() const;

private:
    std::shared_ptr<OuterLoopBendersAdapter> adapter_;  ///< Reference to adapter
};

}  // namespace Outerloop

