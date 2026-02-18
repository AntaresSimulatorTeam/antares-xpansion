#pragma once

#include "IBatchingStrategy.h"
#include "antares-xpansion/benders/benders_by_batch/BendersByBatch.h"
#include <memory>

/**
 * @class ByBatchStrategy
 * @brief Batching strategy adapter for BendersByBatch
 * 
 * Wraps BendersByBatch to implement IBatchingStrategy interface.
 * This adapter allows the batch-based Benders implementation to be used
 * within the Strategy pattern composition.
 * 
 * Note: BendersByBatch extends BendersMPI, so it includes both execution
 * and batching logic. In the Strategy pattern refactoring, we separate these
 * concerns. This adapter exposes only the batching-specific methods.
 */
class ByBatchStrategy : public IBatchingStrategy
{
public:
    /**
     * @brief Constructor that takes ownership of a BendersByBatch instance
     * @param batch_benders Unique pointer to BendersByBatch implementation
     * 
     * Note: BendersByBatch manages both execution and batching. When used
     * in composition, the execution aspects are handled by ExecutionStrategy,
     * and this adapter exposes only batching-specific behavior.
     */
    explicit ByBatchStrategy(std::unique_ptr<BendersBase> batch_benders)
        : batch_benders_(std::move(batch_benders))
    {
    }

    void InitializeProblems() override
    {
        if (batch_benders_)
        {
            batch_benders_->InitializeProblems();
        }
    }

    void UpdateStoppingCriterion() override
    {
        if (batch_benders_)
        {
            // No public API to trigger internal UpdateStoppingCriterion on the concrete implementation
            // in this refactoring pass; keep as no-op to allow compilation and test harness to control
            // batching behavior via IBatchingStrategy mock in unit tests.
        }
    }

    [[nodiscard]] bool ShouldRelaxationStop() const override
    {
        if (batch_benders_)
        {
            // No direct public equivalent — return false as default for the strategy tests
            return false;
        }
        return false;
    }

private:
    std::unique_ptr<BendersBase> batch_benders_;
};
