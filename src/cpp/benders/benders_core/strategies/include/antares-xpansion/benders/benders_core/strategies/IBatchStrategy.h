#pragma once

#include <memory>
#include <string>

class BendersBase;
class BendersCore;

class IBatchStrategy
{
public:
    virtual ~IBatchStrategy() = default;

    virtual void Run(BendersCore& benders) = 0;

    virtual void InitializeProblems(BendersCore& benders) = 0;

    [[nodiscard]] virtual std::string name() const = 0;
};

using BatchStrategyPtr = std::unique_ptr<IBatchStrategy>;
