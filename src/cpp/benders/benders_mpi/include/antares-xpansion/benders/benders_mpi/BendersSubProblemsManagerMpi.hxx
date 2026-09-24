#pragma once

#include "antares-xpansion/benders/benders_core/BendersSubProblemsManager.hxx"
#include "antares-xpansion/benders/benders_core/CriterionComputation.h"

class BendersSubProblemsManagerMpi: public BendersSubProblemsManager<BendersSubProblemsManagerMpi>
{
public:
    using BendersSubProblemsManager::BendersSubProblemsManager;
    using BendersSubProblemsManager::MakePostSolveHookImpl;

    SubProblemNamesInCut DistributeSubproblemsImpl(int rank, int world_size)
    {
        SubProblemNamesInCut subs_per_proc;
        if (options_.CACHE_PROBLEMS != CacheProblems::NO_CACHE)
        {
            int current_problem_id = 0;
            for (auto it = coupling_map_.begin(); it != coupling_map_.end();)
            {
                auto process_to_feed = current_problem_id % world_size;
                if (process_to_feed != rank)
                {
                    it = coupling_map_.erase(it);
                }
                else
                {
                    AddSubproblemName(it->first);
                    subs_per_proc.emplace_back(it->first, process_to_feed);
                    ++it;
                }
                current_problem_id++;
            }
        }
        else
        {
            int current_problem_id = 0;
            for (const auto& problem: coupling_map_)
            {
                if (auto process_to_feed = current_problem_id % world_size; process_to_feed == rank)
                {
                    subs_per_proc.push_back(std::make_pair(problem.first, process_to_feed));
                    AddSubproblem(problem);
                    AddSubproblemName(problem.first);
                }
                current_problem_id++;
            }
        }
        return subs_per_proc;
    }

    auto MakePostSolveHookImpl(Benders::Criterion::CriterionComputation& criterion,
                               CurrentIterationData& data)
    {
        return [this, &criterion, &data](const std::string& name,
                                         PlainData::SubProblemData& subproblem_data,
                                         const SubproblemWorkerPtr& worker)
        {
            std::vector<double> solution = worker->get_solution();
            criterion.ComputeCriterion(SubproblemWeight(data.control.nsubproblem, name),
                                       solution,
                                       subproblem_data.criteria,
                                       subproblem_data.patterns_values);
        };
    }
};
