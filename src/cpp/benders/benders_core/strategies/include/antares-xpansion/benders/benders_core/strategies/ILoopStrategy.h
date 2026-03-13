#pragma once

#include <memory>
#include <string>

#include "antares-xpansion/benders/benders_core/BendersStructsDatas.h"

class BendersBase;

class ILoopStrategy
{
public:
    virtual ~ILoopStrategy() = default;

    virtual void run(BendersBase& benders) = 0;

    [[nodiscard]] virtual bool should_stop(const CurrentIterationData& data) const = 0;

    [[nodiscard]] virtual std::string name() const = 0;
};

using LoopStrategyPtr = std::unique_ptr<ILoopStrategy>;
