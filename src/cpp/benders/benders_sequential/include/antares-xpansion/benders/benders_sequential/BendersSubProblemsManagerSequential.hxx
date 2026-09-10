#pragma once

#include "antares-xpansion/benders/benders_core/BendersSubProblemsManager.hxx"

class BendersSubProblemsManagerSequential
    : public BendersSubProblemsManager<BendersSubProblemsManagerSequential>
{
public:
    using BendersSubProblemsManager::BendersSubProblemsManager;

    // Uses default MakeFastBeginHookImpl() and MakeCacheBeginHookImpl()
    // from the base — iterates all subproblems.
};
