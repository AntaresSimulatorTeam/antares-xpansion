#pragma once

#include "antares-xpansion/benders/benders_core/BendersSubProblemsManager.hxx"

class BendersSubProblemsManagerSequential
    : public BendersSubProblemsManager<BendersSubProblemsManagerSequential>
{
public:
    using BendersSubProblemsManager::BendersSubProblemsManager;

    static constexpr auto EXECUTION_POLICY = std::execution::par;

    // Uses default MakeFastBeginHookImpl() and MakeCacheBeginHookImpl()
    // from the base — iterates all subproblems.
};
