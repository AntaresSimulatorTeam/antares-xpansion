#pragma once

#include "IOuterLoopStrategy.h"
#include "antares-xpansion/benders/outer_loop/OuterLoop.h"
#include <memory>

/**
 * @class OuterLoopAdapter
 * @brief Outer loop strategy adapter for OuterLoop
 * 
 * Wraps Outerloop::OuterLoop to implement IOuterLoopStrategy interface.
 * This adapter allows the outer loop Benders implementation to be used
 * within the Strategy pattern composition.
 * 
 * Note: OuterLoop is an abstract class with pure virtual methods that
 * must be implemented by concrete subclasses. This adapter wraps any
 * concrete OuterLoop implementation.
 */
class OuterLoopAdapter : public IOuterLoopStrategy
{
public:
    /**
     * @brief Constructor that takes ownership of an OuterLoop instance
     * @param outer_loop Unique pointer to a concrete OuterLoop implementation
     */
    explicit OuterLoopAdapter(std::unique_ptr<Outerloop::OuterLoop> outer_loop)
        : outer_loop_(std::move(outer_loop))
    {
    }

    void Run() override
    {
        if (outer_loop_)
        {
            outer_loop_->Run();
        }
    }

    void RunAttachedAlgo() override
    {
        if (outer_loop_)
        {
            outer_loop_->RunAttachedAlgo();
        }
    }

    bool UpdateMaster() override
    {
        return outer_loop_ ? outer_loop_->UpdateMaster() : false;
    }

    void PrintLog() override
    {
        if (outer_loop_)
        {
            outer_loop_->PrintLog();
        }
    }

    void init_data() override
    {
        if (outer_loop_)
        {
            outer_loop_->init_data();
        }
    }

    bool isExceptionRaised() override
    {
        return outer_loop_ ? outer_loop_->isExceptionRaised() : false;
    }

    [[nodiscard]] double OuterLoopLambdaMin() const override
    {
        return outer_loop_ ? outer_loop_->OuterLoopLambdaMin() : 0.0;
    }

    [[nodiscard]] double OuterLoopLambdaMax() const override
    {
        return outer_loop_ ? outer_loop_->OuterLoopLambdaMax() : 0.0;
    }

    void OuterLoopCheckFeasibility() override
    {
        if (outer_loop_)
        {
            outer_loop_->OuterLoopCheckFeasibility();
        }
    }

    void OuterLoopBilevelChecks() override
    {
        if (outer_loop_)
        {
            outer_loop_->OuterLoopBilevelChecks();
        }
    }

private:
    std::unique_ptr<Outerloop::OuterLoop> outer_loop_;
};
