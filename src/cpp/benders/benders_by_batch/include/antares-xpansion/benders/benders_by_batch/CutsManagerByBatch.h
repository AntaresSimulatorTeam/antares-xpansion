#pragma once

#include "antares-xpansion/benders/benders_core/BendersStructsDatas.h"
#include "antares-xpansion/benders/benders_core/CutsManager.hxx"
#include "antares-xpansion/benders/benders_mpi/common_mpi.h"
#include "antares-xpansion/helpers/Timer.h"

class CutsManagerByBatch: public CutsManager<CutsManagerByBatch>
{
public:
    CutsManagerByBatch(mpi::communicator& world,
                       int rank_0,
                       CurrentIterationData& data,
                       const VariableMap& problem_to_id,
                       BendersRelevantIterationsData& relevantIterationData,
                       const WorkerMasterPtr& master);

    void GatherAndBuildCutsImpl(const SubProblemDataMap& subproblem_data_map,
                                const Timer& walltime,
                                const std::vector<SubProblemNamesInCut>& subproblems_per_cut,
                                double& batch_contribution_in_gap);

    void BroadcastXCut()
    {
        mpi::broadcast(world_, data_.x_cut, rank_0_);
    }

private:
    double ComputeBatchContributionInGap(
      const std::vector<SubProblemDataMap>& gathered_subproblem_map,
      const std::vector<SubProblemNamesInCut>& subproblems_per_cut) const;

    mpi::communicator& world_;
    int rank_0_;
    CurrentIterationData& data_;
    const VariableMap& problem_to_id_;
    BendersRelevantIterationsData& relevantIterationData_;
    const WorkerMasterPtr& master_;
};
