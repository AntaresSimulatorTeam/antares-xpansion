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
    explicit ByBatchStrategy(std::unique_ptr<BendersByBatch> batch_benders)
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
            batch_benders_->UpdateStoppingCriterion();
        }
    }

    [[nodiscard]] bool ShouldRelaxationStop() const override
    {
        return batch_benders_ ? batch_benders_->ShouldRelaxationStop() : false;
    }

private:
    std::unique_ptr<BendersByBatch> batch_benders_;
};
