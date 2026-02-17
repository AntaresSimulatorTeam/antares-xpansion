#pragma once

#include "IExecutionStrategy.h"
#include "antares-xpansion/benders/benders_sequential/BendersSequential.h"
#include <memory>

/**
 * @class SequentialExecutionStrategy
 * @brief Execution strategy adapter for BendersSequential
 * 
 * Wraps BendersSequential to implement IExecutionStrategy interface.
 * This adapter allows the sequential Benders implementation to be used
 * within the Strategy pattern composition.
 */
class SequentialExecutionStrategy : public IExecutionStrategy
{
public:
    /**
     * @brief Constructor that takes ownership of a BendersSequential instance
     * @param sequential Unique pointer to BendersSequential implementation
     */
    explicit SequentialExecutionStrategy(std::unique_ptr<BendersSequential> sequential)
        : sequential_(std::move(sequential))
    {
    }

    void launch() override
    {
        if (sequential_)
        {
            sequential_->launch();
        }
    }

    void InitializeProblems() override
    {
        if (sequential_)
        {
            sequential_->InitializeProblems();
        }
    }

    void Run() override
    {
        // BendersSequential's Run() is protected, but launch() calls it internally
        // For now, delegate to launch() which handles the full execution
        if (sequential_)
        {
            sequential_->launch();
        }
    }

    [[nodiscard]] std::string BendersName() const override
    {
        return sequential_ ? sequential_->BendersName() : "SequentialExecutionStrategy";
    }

    [[nodiscard]] double execution_time() const override
    {
        return sequential_ ? sequential_->execution_time() : 0.0;
    }

private:
    std::unique_ptr<BendersSequential> sequential_;
};
