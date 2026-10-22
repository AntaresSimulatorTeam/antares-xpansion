#pragma once

#include "antares-xpansion/benders/benders_by_batch/BatchCollection.h"
#include "antares-xpansion/benders/benders_core/BendersSubProblemsManager.hxx"
#include "antares-xpansion/xpansion_interfaces/LogUtils.h"

class BendersSubProblemsManagerByBatch
    : public BendersSubProblemsManager<BendersSubProblemsManagerByBatch>
{
public:
    using BendersSubProblemsManager::BendersSubProblemsManager;

    static constexpr auto EXECUTION_POLICY = std::execution::seq;

    // Default hooks (no batch filter) — inherited from base
    using BendersSubProblemsManager::MakeCacheBeginHookImpl;
    using BendersSubProblemsManager::MakeFastBeginHookImpl;
    using BendersSubProblemsManager::MakePostSolveHookImpl;

    void DistributeSubproblemsImpl(BatchCollection& batch_collection, int rank, int world_size)
    {
        // Without cache, the solver of each subproblem owned by this process is created once
        // and kept; with a cache, workers are rebuilt at each iteration so only the name is kept.
        const bool keep_workers = options_.CACHE_PROBLEMS == CacheProblems::NO_CACHE;
        KeepOnlySubproblemsOfRank(batch_collection,
                                  rank,
                                  world_size,
                                  [this, keep_workers](const std::string& name)
                                  {
                                      if (keep_workers)
                                      {
                                          AddSubproblem({name, coupling_map_[name]});
                                      }
                                      AddSubproblemName(name);
                                  });
    }

    // Batch post-solve: adapts a callback taking (name, data) to the 3-arg PostSolveHook signature
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
            std::vector<std::pair<std::string, SubproblemWorkerPtr>> nameAndWorkers;
            nameAndWorkers.reserve(batch_sub_problems.size());
            for (const auto& name: batch_sub_problems)
            {
                nameAndWorkers.emplace_back(name,
                                            find_or_throw(subproblem_map_, name, "subproblem map"));
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
                nameAndVariableMap.emplace_back(name,
                                                find_or_throw(coupling_map_, name, "coupling map"));
            }
            return nameAndVariableMap;
        };
    }

private:
    // Subproblems are dealt round-robin over the processes, following the batch order. Removes
    // from each batch the subproblems that do not belong to this rank, and calls on_owned on
    // the ones that do.
    template<typename OnOwned>
    static void KeepOnlySubproblemsOfRank(BatchCollection& batch_collection,
                                          int rank,
                                          int world_size,
                                          OnOwned&& on_owned)
    {
        auto problem_count = 0;
        for (auto& batch: batch_collection.BatchCollections())
        {
            for (auto it = batch.sub_problem_names.begin(); it != batch.sub_problem_names.end();)
            {
                if (problem_count % world_size != rank)
                {
                    it = batch.sub_problem_names.erase(it);
                }
                else
                {
                    on_owned(*it);
                    ++it;
                }
                ++problem_count;
            }
            batch.sub_problem_names.shrink_to_fit();
        }
    }

    template<typename Map>
    static const typename Map::mapped_type& find_or_throw(const Map& map,
                                                          const std::string& name,
                                                          const std::string& map_description)
    {
        const auto it = map.find(name);
        if (it == map.end())
        {
            throw LogUtils::XpansionError<std::runtime_error>(
              "Subproblem " + name + " of the current batch is not in the " + map_description
                + " of this process",
              LOGLOCATION);
        }
        return it->second;
    }
};
