#include "antares-xpansion/benders/benders_by_batch/BendersCutsManagerByBatch.h"

#include <numeric>

BendersCutsManagerByBatch::BendersCutsManagerByBatch(
  mpi::communicator& world,
  int rank_0,
  CurrentIterationData& data,
  const VariableMap& problem_to_id,
  BendersRelevantIterationsData& relevantIterationData,
  const WorkerMasterPtr& master):
    world_(world),
    rank_0_(rank_0),
    data_(data),
    problem_to_id_(problem_to_id),
    relevantIterationData_(relevantIterationData),
    master_(master)
{
}

void BendersCutsManagerByBatch::GatherAndBuildCutsImpl(
  const SubProblemDataMap& subproblem_data_map,
  const Timer& walltime,
  const std::vector<SubProblemNamesInCut>& subproblems_per_cut,
  double& batch_contribution_in_gap)
{
    std::vector<SubProblemDataMap> gathered_subproblem_map;
    mpi::gather(world_, subproblem_data_map, gathered_subproblem_map, rank_0_);
    data_.subproblems_walltime = walltime.elapsed();

    SetSubproblemDataCostAndSimplexIter(gathered_subproblem_map, data_);

    if (world_.rank() == rank_0_)
    {
        batch_contribution_in_gap = ComputeBatchContributionInGap(gathered_subproblem_map,
                                                                  subproblems_per_cut);
        BuildAllAggregatedCuts(subproblems_per_cut,
                               gathered_subproblem_map,
                               problem_to_id_,
                               data_.ub,
                               data_.x_cut,
                               relevantIterationData_.last._cut_trace,
                               master_);
    }
}

double BendersCutsManagerByBatch::ComputeBatchContributionInGap(
  const std::vector<SubProblemDataMap>& gathered_subproblem_map,
  const std::vector<SubProblemNamesInCut>& subproblems_per_cut) const
{
    double batch_contribution_in_gap = 0.0;
    for (const auto& names_and_positions_in_gathered: subproblems_per_cut)
    {
        // Performs max(0, sum_{s sub_pb in cut}(phi_s(x) - theta_s))
        // where phi_s(x) - theta_s has already been computed within each proc and is equal to
        // contribution_in_gap
        double sum = std::accumulate(
          names_and_positions_in_gathered.begin(),
          names_and_positions_in_gathered.end(),
          0.0,
          [&](double acc, const auto& name_and_position)
          {
              const auto& subproblem_name = name_and_position.first;
              size_t pos = name_and_position.second;
              return acc + gathered_subproblem_map[pos].at(subproblem_name).contribution_in_gap;
          });
        batch_contribution_in_gap += std::max(0.0, sum);
    }
    return batch_contribution_in_gap;
}
