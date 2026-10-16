#pragma once

#include "antares-xpansion/benders/benders_by_batch/BatchCollection.h"
#include "antares-xpansion/benders/benders_core/BendersCutsManager.hxx"
#include "antares-xpansion/benders/benders_core/BendersStructsDatas.h"
#include "antares-xpansion/benders/benders_mpi/common_mpi.h"
#include "antares-xpansion/helpers/Timer.h"

class BendersCutsManagerByBatch: public BendersCutsManager<BendersCutsManagerByBatch>
{
public:
    BendersCutsManagerByBatch(mpi::communicator& world,
                              int rank_0,
                              CurrentIterationData& data,
                              const VariableMap& problem_to_id,
                              BendersRelevantIterationsData& relevantIterationData,
                              BendersMasterManager& master_manager,
                              const BatchCollection& batch_collection_for_cuts);

    void GatherAndBuildCutsImpl(const SubProblemDataMap& subproblem_data_map,
                                const Timer& walltime,
                                unsigned batch_id,
                                double& batch_contribution_in_gap);

    void BroadcastXCut()
    {
        mpi::broadcast(world_, data_.solution.x_cut, rank_0_);
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
    // Only filled on rank_0_, the only rank that builds cuts
    const BatchCollection& batch_collection_for_cuts_;
};
