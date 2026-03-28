

#include "antares-xpansion/benders/benders_mpi/BendersMPI.h"

#include <utility>

#include "antares-xpansion/benders/benders_core/BendersProblemFromFile.h"
#include "antares-xpansion/benders/benders_core/CriterionComputation.h"
#include "antares-xpansion/helpers/Timer.h"

BendersMpi::BendersMpi(const BendersBaseOptions& options,
                       std::shared_ptr<ILogger> logger,
                       std::shared_ptr<Output::OutputWriter> writer,
                       mpi::communicator& world,
                       std::shared_ptr<MathLoggerDriver> mathLoggerDriver):
    BendersBase(options,
                std::move(logger),
                std::move(writer),
                std::move(mathLoggerDriver),
                std::make_shared<MpiCommunicationStrategy>(world)),
    _world(world)
{
}

/*!
 *  \brief Method to load each problem in a thread
 *
 *  The initialization of each problem is done sequentially
 *
 */

void BendersMpi::InitializeProblems()
{
    MatchProblemToId();
    SubProblemNamesInCut subs_per_proc;
    if (_options.CACHE_PROBLEMS)
    {
        int current_problem_id = 0;
        for (auto it = coupling_map_.begin(); it != coupling_map_.end();)
        {
            auto process_to_feed = current_problem_id % _world.size();
            if (process_to_feed != _world.rank())
            {
                it = coupling_map_.erase(it);
            }
            else
            {
                subs_per_proc.emplace_back(it->first, process_to_feed);
                ++it;
            }
            current_problem_id++;
        }
    }
    else
    {
        int current_problem_id = 0;
        // Dispatch subproblems to process
        for (const auto& problem: coupling_map_)
        {
            // In case there are more subproblems than process
            if (auto process_to_feed = current_problem_id % _world.size();
                process_to_feed == _world.rank())
            { // Assign  [problemNumber % processCount] to processID
                subs_per_proc.push_back(std::make_pair(problem.first, process_to_feed));
                AddSubproblem(problem);
                AddSubproblemName(problem.first);
            }
            current_problem_id++;
        }
    }

    std::vector<SubProblemNamesInCut> gathered_subs_per_proc;
    mpi::gather(_world, subs_per_proc, gathered_subs_per_proc, rank_0);
    if (_world.rank() == rank_0)
    {
        subproblem_per_cut_indices_ = get_subs_per_cut(gathered_subs_per_proc, _data.nsubproblem);
    }
    BuildMasterProblem();
    BroadCastVariablesIndices();
    init_problems_ = false;
}

void BendersMpi::BroadCastVariablesIndices()
{
    if (_world.rank() == rank_0)
    {
        SetSubproblemsVariablesIndices();
    }
    BroadCast(criterion_computation_.getVarIndices(), rank_0);
}

void BendersMpi::InitializeMaster()
{
    if (_world.rank() == rank_0)
    {
        std::shared_ptr<IBendersProblemProvider>
          benders_problem_provider = std::make_shared<BendersProblemFromFile>(get_master_path());
        reset_master<WorkerMaster>(master_variable_map_,
                                   get_solver_name(),
                                   get_log_level(),
                                   _data.nsubproblem,
                                   solver_log_manager_,
                                   IsResumeMode(),
                                   _logger,
                                   Options().PROBLEMS_FORMAT,
                                   benders_problem_provider.get(),
                                   Options().MASTER_SOLUTION_TOLERANCE,
                                   Options().CUT_COEFFICIENT_TOLERANCE);
    }
}

void BendersMpi::BuildMasterProblem()
{
    InitializeMaster();
    if (_world.rank() == rank_0)
    {
        _master->addAlphasFixingConstraints(subproblem_per_cut_indices_, _problem_to_id);
    }
}

/*!
 *  \brief Method to free the memory used by each problem
 */
void BendersMpi::free()
{
    if (_world.rank() == rank_0)
    {
        free_master();
    }
    else
    {
        free_subproblems();
    }
    _world.barrier();
}

void BendersMpi::SolveSubproblem(PlainData::SubProblemData& subproblem_data,
                                 const std::string& name,
                                 const std::shared_ptr<SubproblemWorker>& worker)
{
    BendersBase::SolveSubproblem(subproblem_data, name, worker);

    std::vector<double> solution = worker->get_solution();
    criterion_computation_.ComputeCriterion(SubproblemWeight(_data.nsubproblem, name),
                                            solution,
                                            subproblem_data.criteria,
                                            subproblem_data.patterns_values);
}
