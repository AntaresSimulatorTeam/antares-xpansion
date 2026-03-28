#include "antares-xpansion/benders/benders_sequential/BendersSequential.h"

#include <algorithm>
#include <iomanip>
#include <utility>

#include "antares-xpansion/benders/benders_core/BendersProblemFromFile.h"
#include "antares-xpansion/benders/benders_core/SequentialCommunicationStrategy.h"
#include "antares-xpansion/helpers/Timer.h"
#include "antares-xpansion/helpers/solver_utils.h"

/*!
 *  \brief Constructor of class BendersSequential
 *
 *  Method to build a BendersSequential element, initializing each problem from
 * a list
 *
 *  \param options : set of options fixed by the user
 */

BendersSequential::BendersSequential(const BendersBaseOptions& options,
                                     Logger logger,
                                     std::shared_ptr<Output::OutputWriter> writer,
                                     std::shared_ptr<MathLoggerDriver> mathLoggerDriver):
    BendersBase(options,
                std::move(logger),
                std::move(writer),
                mathLoggerDriver,
                std::make_shared<SequentialCommunicationStrategy>())
{
}

void BendersSequential::InitializeProblems()
{
    MatchProblemToId();
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
    SubProblemNamesInCut subs_per_proc;
    for (const auto& problem: coupling_map_)
    {
        subs_per_proc.emplace_back(problem.first, 0);
        AddSubproblem(problem);
        AddSubproblemName(problem.first);
    }
    subproblem_per_cut_indices_ = get_subs_per_cut({subs_per_proc}, _data.nsubproblem);
}

/*!
 *  \brief Method to free the memory used by each problem
 */
void BendersSequential::free()
{
    if (get_master())
    {
        free_master();
    }
    free_subproblems();
}
