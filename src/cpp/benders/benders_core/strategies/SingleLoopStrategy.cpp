#include "antares-xpansion/benders/benders_core/strategies/SingleLoopStrategy.h"

#include "antares-xpansion/benders/benders_core/BendersBase.h"

SingleLoopStrategy::SingleLoopStrategy() = default;

void SingleLoopStrategy::run(BendersBase& benders)
{
}

bool SingleLoopStrategy::should_stop(const CurrentIterationData& data) const
{
    return data.stop;
}

std::string SingleLoopStrategy::name() const
{
    return "Single Loop Strategy";
}
