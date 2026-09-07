#pragma once

#include "antares-xpansion/benders/benders_core/BendersStructsDatas.h"
#include "antares-xpansion/benders/benders_core/BendersCutsManager.hxx"

class BendersCutsManagerSequential: public BendersCutsManager<BendersCutsManagerSequential>
{
public:
    BendersCutsManagerSequential(CurrentIterationData& data,
                          VariableMap& problem_to_id,
                          BendersRelevantIterationsData& relevantIterationData,
                          const WorkerMasterPtr& master,
                          int nb_cuts_per_iter);

    void GatherAndBuildCutsImpl(const SubProblemDataMap& subproblem_data_map);

private:
    CurrentIterationData& data_;
    VariableMap& problem_to_id_;
    BendersRelevantIterationsData& relevantIterationData_;
    const WorkerMasterPtr& master_;
    int nb_cuts_per_iter_;
};
