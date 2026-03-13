#include "antares-xpansion/benders/benders_core/strategies/OuterLoopStrategy.h"

#include "antares-xpansion/benders/benders_core/BendersBase.h"

OuterLoopStrategy::OuterLoopStrategy() = default;

void OuterLoopStrategy::run(BendersBase& benders)
{
}

bool OuterLoopStrategy::should_stop(const CurrentIterationData& data) const
{
    return data.stop;
}

std::string OuterLoopStrategy::name() const
{
    return "Outer Loop Strategy";
}
