#pragma once

#include "antares-xpansion/benders/benders_core/BendersCutsManager.hxx"
#include "antares-xpansion/benders/benders_core/BendersStructsDatas.h"

class BendersCutsManagerSequential: public BendersCutsManager<BendersCutsManagerSequential>
{
public:
    BendersCutsManagerSequential(CurrentIterationData& data,
                                 VariableMap& problem_to_id,
                                 BendersRelevantIterationsData& relevantIterationData,
                                 const WorkerMasterPtr& master);

    void GatherAndBuildCutsImpl(const SubProblemDataMap& subproblem_data_map);
    void SetSubproblemPerCutIndices(std::vector<SubProblemNamesInCut> indices);

private:
    CurrentIterationData& data_;
    VariableMap& problem_to_id_;
    BendersRelevantIterationsData& relevantIterationData_;
    const WorkerMasterPtr& master_;
    std::vector<SubProblemNamesInCut> subproblem_per_cut_indices_;
};
