#pragma once

#include "IBatchingStrategy.h"

/**
 * @class NoBatchingStrategy
 * @brief No-op batching strategy (passthrough)
 *
 * This strategy implements the IBatchingStrategy interface without
 * performing any actual batching. It's used when batching is not required.
 * All methods are no-ops or return safe defaults.
 */
class NoBatchingStrategy: public IBatchingStrategy
{
public:
    /**
     * @brief Default constructor
     */
    NoBatchingStrategy() = default;

    /**
     * @brief Virtual destructor
     */
    ~NoBatchingStrategy() override = default;

    /**
     * @brief No-op initialization (batching not used)
     */
    void InitializeProblems() override
    {
        // No batching initialization needed
    }

    /**
     * @brief No-op stopping criterion update (batching not used)
     */
    void UpdateStoppingCriterion() override
    {
        // No batching-specific stopping criterion to update
    }

    /**
     * @brief Always returns false (no batching relaxation)
     * @return false - relaxation should not stop due to batching
     */
    [[nodiscard]] bool ShouldRelaxationStop() const override
    {
        return false; // No batching, so never stop relaxation for batch reasons
    }
};
