#pragma once

#include "antares-xpansion/benders/benders_core/BendersSubProblemsManager.hxx"

class BendersSubProblemsManagerByBatch
    : public BendersSubProblemsManager<BendersSubProblemsManagerByBatch>
{
public:
    using BendersSubProblemsManager::BendersSubProblemsManager;

    // Default hooks (no batch filter) — inherited from base
    using BendersSubProblemsManager::MakeCacheBeginHookImpl;
    using BendersSubProblemsManager::MakeFastBeginHookImpl;

    // Batch-scoped overrides: only include subproblems in the current batch
    FastBeginHook MakeFastBeginHookImpl(const std::vector<std::string>& batch_sub_problems)
    {
        return [this, &batch_sub_problems]()
        {
            const auto& sub_pblm_map = GetSubProblemMap();
            std::vector<std::pair<std::string, SubproblemWorkerPtr>> nameAndWorkers;
            nameAndWorkers.reserve(batch_sub_problems.size());
            for (const auto& name: batch_sub_problems)
            {
                auto it = sub_pblm_map.find(name);
                nameAndWorkers.emplace_back(it->first, it->second);
            }
            return nameAndWorkers;
        };
    }

    CacheBeginHook MakeCacheBeginHookImpl(const std::vector<std::string>& batch_sub_problems)
    {
        return [this, &batch_sub_problems]()
        {
            std::vector<std::pair<std::string, VariableMap>> nameAndVariableMap;
            nameAndVariableMap.reserve(batch_sub_problems.size());
            for (const auto& name: batch_sub_problems)
            {
                auto it = coupling_map_.find(name);
                nameAndVariableMap.emplace_back(it->first, it->second);
            }
            return nameAndVariableMap;
        };
    }
};
