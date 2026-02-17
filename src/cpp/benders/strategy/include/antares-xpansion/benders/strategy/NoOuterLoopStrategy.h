#pragma once

#include "IOuterLoopStrategy.h"

/**
 * @class NoOuterLoopStrategy
 * @brief No-op outer loop strategy (passthrough)
 * 
 * This strategy implements the IOuterLoopStrategy interface without
 * performing any actual outer loop logic. It's used when outer loop
 * optimization is not required. All methods are no-ops or return safe defaults.
 */
class NoOuterLoopStrategy : public IOuterLoopStrategy
{
public:
    /**
     * @brief Default constructor
     */
    NoOuterLoopStrategy() = default;
    
    /**
     * @brief Virtual destructor
     */
    ~NoOuterLoopStrategy() override = default;

    /**
     * @brief No-op Run (outer loop not used)
     */
    void Run() override
    {
        // No outer loop to run
    }

    /**
     * @brief No-op RunAttachedAlgo (outer loop not used)
     */
    void RunAttachedAlgo() override
    {
        // No attached algorithm to run
    }

    /**
     * @brief No-op UpdateMaster (outer loop not used)
     * @return false - no master update performed
     */
    bool UpdateMaster() override
    {
        return false;  // No outer loop, so no master update
    }

    /**
     * @brief No-op PrintLog (outer loop not used)
     */
    void PrintLog() override
    {
        // No outer loop logs to print
    }

    /**
     * @brief No-op init_data (outer loop not used)
     */
    void init_data() override
    {
        // No outer loop data to initialize
    }

    /**
     * @brief No exceptions raised (outer loop not used)
     * @return false - no exceptions
     */
    bool isExceptionRaised() override
    {
        return false;  // No outer loop, so no exceptions
    }

    /**
     * @brief Lambda min (not applicable without outer loop)
     * @return 0.0 - default value
     */
    [[nodiscard]] double OuterLoopLambdaMin() const override
    {
        return 0.0;  // No outer loop, return default
    }

    /**
     * @brief Lambda max (not applicable without outer loop)
     * @return 0.0 - default value
     */
    [[nodiscard]] double OuterLoopLambdaMax() const override
    {
        return 0.0;  // No outer loop, return default
    }

    /**
     * @brief No-op feasibility check (outer loop not used)
     */
    void OuterLoopCheckFeasibility() override
    {
        // No outer loop feasibility to check
    }

    /**
     * @brief No-op bilevel checks (outer loop not used)
     */
    void OuterLoopBilevelChecks() override
    {
        // No outer loop bilevel checks to perform
    }
};
