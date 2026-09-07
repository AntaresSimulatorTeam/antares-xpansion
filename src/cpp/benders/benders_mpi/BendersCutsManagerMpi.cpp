#include "antares-xpansion/benders/benders_mpi/BendersCutsManagerMpi.h"

BendersCutsManagerMpi::BendersCutsManagerMpi(mpi::communicator& world,
                               int rank_0,
                               CurrentIterationData& data,
                               const VariableMap& problem_to_id,
                               BendersRelevantIterationsData& relevantIterationData,
                               const WorkerMasterPtr& master,
                               const std::vector<SubProblemNamesInCut>& subproblem_per_cut_indices,
                               Logger logger):
    world_(world),
    rank_0_(rank_0),
    data_(data),
    problem_to_id_(problem_to_id),
    relevantIterationData_(relevantIterationData),
    master_(master),
    subproblem_per_cut_indices_(subproblem_per_cut_indices),
    logger_(std::move(logger))
{
}

void BendersCutsManagerMpi::GatherAndBuildCutsImpl(const SubProblemDataMap& subproblem_data_map,
                                            const Timer& walltime,
                                            bool exception_raised)
{
    if (!exception_raised)
    {
        GatherCuts(subproblem_data_map, walltime);
    }
}

void BendersCutsManagerMpi::GatherCuts(const SubProblemDataMap& subproblem_data_map, const Timer& walltime)
{
    std::vector<SubProblemDataMap> gathered_subproblem_map;
    mpi::gather(world_, subproblem_data_map, gathered_subproblem_map, rank_0_);
    data_.subproblems_walltime = walltime.elapsed();
    double cumulative_subproblems_timer_per_iter(0);
    mpi::reduce(world_,
                data_.subproblems_cputime,
                cumulative_subproblems_timer_per_iter,
                std::plus<double>(),
                rank_0_);
    data_.subproblems_cumulative_cputime = cumulative_subproblems_timer_per_iter;

    MasterBuildCuts(gathered_subproblem_map);
}

void BendersCutsManagerMpi::MasterBuildCuts(const std::vector<SubProblemDataMap>& gathered_subproblem_map)
{
    data_.subproblem_cost = 0;
    SetSubproblemDataCostAndSimplexIter(gathered_subproblem_map, data_);

    data_.ub = 0;

    if (world_.rank() == rank_0_)
    {
        BuildAllAggregatedCuts(subproblem_per_cut_indices_,
                               gathered_subproblem_map,
                               problem_to_id_,
                               data_.ub,
                               data_.x_cut,
                               relevantIterationData_.last._cut_trace,
                               master_);
    }

    logger_->LogSubproblemsSolvingCumulativeCpuTime(data_.subproblems_cumulative_cputime);
    logger_->LogSubproblemsSolvingWalltime(data_.subproblems_walltime);
}
