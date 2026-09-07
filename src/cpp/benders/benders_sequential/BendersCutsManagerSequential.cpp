#include "antares-xpansion/benders/benders_sequential/BendersCutsManagerSequential.h"

BendersCutsManagerSequential::BendersCutsManagerSequential(
  CurrentIterationData& data,
  VariableMap& problem_to_id,
  BendersRelevantIterationsData& relevantIterationData,
  const WorkerMasterPtr& master,
  int nb_cuts_per_iter):
    data_(data),
    problem_to_id_(problem_to_id),
    relevantIterationData_(relevantIterationData),
    master_(master),
    nb_cuts_per_iter_(nb_cuts_per_iter)
{
}

void BendersCutsManagerSequential::GatherAndBuildCutsImpl(
  const SubProblemDataMap& subproblem_data_map)
{
    data_.ub = 0;

    if (nb_cuts_per_iter_)
    {
        ComputeCutAggregate(subproblem_data_map,
                            data_.ub,
                            data_.x_cut,
                            relevantIterationData_.last._cut_trace,
                            master_);
    }
    else
    {
        ComputeCut(subproblem_data_map,
                   data_.ub,
                   data_.x_cut,
                   problem_to_id_,
                   relevantIterationData_.last._cut_trace,
                   master_);
    }
}
