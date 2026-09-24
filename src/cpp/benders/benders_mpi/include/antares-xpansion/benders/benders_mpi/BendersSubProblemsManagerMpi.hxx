#pragma once

#include "antares-xpansion/benders/benders_core/BendersSubProblemsManager.hxx"
#include "antares-xpansion/benders/benders_core/CriterionComputation.h"

class BendersSubProblemsManagerMpi: public BendersSubProblemsManager<BendersSubProblemsManagerMpi>
{
public:
    using BendersSubProblemsManager::BendersSubProblemsManager;
    using BendersSubProblemsManager::MakePostSolveHookImpl;

    auto MakePostSolveHookImpl(Benders::Criterion::CriterionComputation& criterion,
                               CurrentIterationData& data)
    {
        return [this, &criterion, &data](const std::string& name,
                                         PlainData::SubProblemData& subproblem_data,
                                         const SubproblemWorkerPtr& worker)
        {
            std::vector<double> solution = worker->get_solution();
            criterion.ComputeCriterion(SubproblemWeight(data.nsubproblem, name),
                                       solution,
                                       subproblem_data.criteria,
                                       subproblem_data.patterns_values);
        };
    }
};
