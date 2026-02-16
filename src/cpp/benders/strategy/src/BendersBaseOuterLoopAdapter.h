#pragma once

#include "antares-xpansion/benders/strategy/IOuterLoopStrategy.h"
#include "antares-xpansion/benders/outer_loop/OuterLoop.h"

class BendersBaseOuterLoopAdapter : public IOuterLoopStrategy
{
public:
    explicit BendersBaseOuterLoopAdapter(std::shared_ptr<OuterLoop> outer_loop)
        : outer_loop_(std::move(outer_loop))
    {
    }

    void Run() override { outer_loop_->Run(); }
    void RunAttachedAlgo() override { outer_loop_->RunAttachedAlgo(); }
    bool UpdateMaster() override { return outer_loop_->UpdateMaster(); }
    void PrintLog() override { outer_loop_->PrintLog(); }
    void init_data() override { outer_loop_->init_data(); }
    bool isExceptionRaised() override { return outer_loop_->isExceptionRaised(); }
    double OuterLoopLambdaMin() const override { return outer_loop_->OuterLoopLambdaMin(); }
    double OuterLoopLambdaMax() const override { return outer_loop_->OuterLoopLambdaMax(); }
    void OuterLoopCheckFeasibility() override { outer_loop_->OuterLoopCheckFeasibility(); }
    void OuterLoopBilevelChecks() override { outer_loop_->OuterLoopBilevelChecks(); }

private:
    std::shared_ptr<OuterLoop> outer_loop_;
};

