#pragma once

#include <functional>

#include "antares-xpansion/benders/benders_core/IOuterLoopStrategy.h"

class NoOuterLoopStrategy: public IOuterLoopStrategy
{
public:
    explicit NoOuterLoopStrategy(std::function<void()> run_algo):
        run_algo_(run_algo)
    {
    }

    void Run() override
    {
        run_algo_();
    }

private:
    std::function<void()> run_algo_;
};
