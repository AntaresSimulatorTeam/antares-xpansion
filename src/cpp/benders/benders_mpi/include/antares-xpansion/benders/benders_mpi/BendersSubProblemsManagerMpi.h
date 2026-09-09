#pragma once

#include "antares-xpansion/benders/benders_core/BendersSubProblemsManager.hxx"

class BendersSubProblemsManagerMpi: public BendersSubProblemsManager<BendersSubProblemsManagerMpi>
{
public:
    using BendersSubProblemsManager::BendersSubProblemsManager;

    // Uses default MakeFastBeginHookImpl() and MakeCacheBeginHookImpl()
    // from the base — iterates all rank-local subproblems (pruned at init).
};
