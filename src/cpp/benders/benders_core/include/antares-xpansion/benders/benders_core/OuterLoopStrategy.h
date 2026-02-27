#pragma once

#include <memory>

#include "antares-xpansion/benders/benders_core/IOuterLoopStrategy.h"
#include "antares-xpansion/benders/outer_loop/OuterLoop.h"

class OuterLoopStrategy: public IOuterLoopStrategy
{
public:
    explicit OuterLoopStrategy(std::shared_ptr<Outerloop::OuterLoop> outer_loop):
        outer_loop_(std::move(outer_loop))
    {
    }

    void Run() override
    {
        outer_loop_->Run();
    }

private:
    std::shared_ptr<Outerloop::OuterLoop> outer_loop_;
};
