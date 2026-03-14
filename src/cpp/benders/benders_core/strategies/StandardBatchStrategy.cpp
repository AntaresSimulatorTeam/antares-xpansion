#include "antares-xpansion/benders/benders_core/strategies/StandardBatchStrategy.h"

#include "antares-xpansion/benders/benders_core/BendersCore.h"

void StandardBatchStrategy::Run(BendersCore& benders)
{
    benders.RunCore();
}

void StandardBatchStrategy::InitializeProblems(BendersCore& benders)
{
    benders.InitializeProblems();
}

std::string StandardBatchStrategy::name() const
{
    return "Standard Batch Strategy";
}
