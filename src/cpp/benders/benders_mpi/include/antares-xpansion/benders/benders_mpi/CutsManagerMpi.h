#pragma once

#include "antares-xpansion/benders/benders_core/BendersStructsDatas.h"
#include "antares-xpansion/benders/benders_core/CutsManager.hxx"
#include "antares-xpansion/helpers/Timer.h"
#include "antares-xpansion/xpansion_interfaces/ILogger.h"
#include "common_mpi.h"

class CutsManagerMpi: public CutsManager<CutsManagerMpi>
{
public:
    CutsManagerMpi(mpi::communicator& world,
                   int rank_0,
                   CurrentIterationData& data,
                   const VariableMap& problem_to_id,
                   BendersRelevantIterationsData& relevantIterationData,
                   const WorkerMasterPtr& master,
                   const std::vector<SubProblemNamesInCut>& subproblem_per_cut_indices,
                   Logger logger);

    void GatherAndBuildCutsImpl(const SubProblemDataMap& subproblem_data_map,
                                const Timer& walltime,
                                bool exception_raised);

private:
    void GatherCuts(const SubProblemDataMap& subproblem_data_map, const Timer& walltime);
    void MasterBuildCuts(const std::vector<SubProblemDataMap>& gathered_subproblem_map);

    mpi::communicator& world_;
    int rank_0_;
    CurrentIterationData& data_;
    const VariableMap& problem_to_id_;
    BendersRelevantIterationsData& relevantIterationData_;
    const WorkerMasterPtr& master_;
    const std::vector<SubProblemNamesInCut>& subproblem_per_cut_indices_;
    Logger logger_;
};
