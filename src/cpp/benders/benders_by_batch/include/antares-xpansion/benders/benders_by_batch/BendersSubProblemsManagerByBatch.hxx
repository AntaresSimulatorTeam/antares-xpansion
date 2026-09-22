#pragma once

#include "antares-xpansion/benders/benders_by_batch/BatchCollection.h"
#include "antares-xpansion/benders/benders_core/BendersSubProblemsManager.hxx"

class BendersSubProblemsManagerByBatch
    : public BendersSubProblemsManager<BendersSubProblemsManagerByBatch>
{
public:
    using BendersSubProblemsManager::BendersSubProblemsManager;

    // Default hooks (no batch filter) — inherited from base
    using BendersSubProblemsManager::MakeCacheBeginHookImpl;
    using BendersSubProblemsManager::MakeFastBeginHookImpl;
    using BendersSubProblemsManager::MakePostSolveHookImpl;

    void DistributeSubproblemsImpl(BatchCollection& batch_collection, int rank, int world_size)
    {
        auto problem_count = 0;
        for (auto& batch: batch_collection.BatchCollections())
        {
            switch (options_.CACHE_PROBLEMS)
            {
            case 1:
            case 2:
            {
                for (auto it = batch.sub_problem_names.begin();
                     it != batch.sub_problem_names.end();)
                {
                    auto process_to_feed = problem_count % world_size;
                    if (process_to_feed != rank)
                    {
                        it = batch.sub_problem_names.erase(it);
                    }
                    else
                    {
                        AddSubproblemName(*it);
                        ++it;
                    }
                    ++problem_count;
                }
                batch.sub_problem_names.shrink_to_fit();
                break;
            }
            case 0:
            default:
            {
                for (auto it = batch.sub_problem_names.begin();
                     it != batch.sub_problem_names.end();)
                {
                    auto process_to_feed = problem_count % world_size;
                    if (process_to_feed != rank)
                    {
                        it = batch.sub_problem_names.erase(it);
                    }
                    else
                    {
                        AddSubproblem({*it, coupling_map_[*it]});
                        AddSubproblemName(*it);
                        ++it;
                    }
                    ++problem_count;
                }
                batch.sub_problem_names.shrink_to_fit();
                break;
            }
            }
        }
    }

    // Batch post-solve: wraps an external callback with the 3-arg signature
    // i'm guessing too complicated ?? it's just a move of cb that call itself
    auto MakePostSolveHookImpl(
      std::function<void(const std::string&, PlainData::SubProblemData&)> callback)
    {
        return [cb = std::move(callback)](const std::string& name,
                                          PlainData::SubProblemData& data,
                                          const SubproblemWorkerPtr&) { cb(name, data); };
    }

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
