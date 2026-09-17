#include "antares-xpansion/benders/benders_sequential/BendersCutsManagerSequential.h"

BendersCutsManagerSequential::BendersCutsManagerSequential(
  CurrentIterationData& data,
  VariableMap& problem_to_id,
  BendersRelevantIterationsData& relevantIterationData,
  const WorkerMasterPtr& master):
    data_(data),
    problem_to_id_(problem_to_id),
    relevantIterationData_(relevantIterationData),
    master_(master)
{
}

void BendersCutsManagerSequential::GatherAndBuildCutsImpl(
  const SubProblemDataMap& subproblem_data_map)
{
    data_.ub = 0;
    std::vector<SubProblemDataMap> gathered{subproblem_data_map};
    BuildAllAggregatedCuts(subproblem_per_cut_indices_,
                           gathered,
                           problem_to_id_,
                           data_.ub,
                           data_.x_cut,
                           relevantIterationData_.last._cut_trace,
                           master_);
}

void BendersCutsManagerSequential::SetSubproblemPerCutIndices(
  std::vector<SubProblemNamesInCut> indices)
{
    subproblem_per_cut_indices_ = std::move(indices);
}
