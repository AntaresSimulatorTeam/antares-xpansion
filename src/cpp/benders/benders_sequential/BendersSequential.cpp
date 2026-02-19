#include "antares-xpansion/benders/benders_sequential/BendersSequential.h"

#include <algorithm>
#include <iomanip>
#include <utility>

#include "antares-xpansion/benders/benders_core/BendersProblemFromFile.h"
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
    BendersBase(options, std::move(logger), std::move(writer), mathLoggerDriver)
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
    for (const auto& problem: coupling_map_)
    {
        const auto subProblemFilePath = GetSubproblemPath(problem.first);

        AddSubproblem(problem);
        AddSubproblemName(problem.first);
    }
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

/*!
 * \brief Build subproblem cut and store it in the BendersSequential trace
 *
 * Method to build subproblem cuts, store them in the BendersSequential trace
 * and add them to the Master problem
 *
 */
void BendersSequential::BuildCut()
{
    SubProblemDataMap subproblem_data_map;
    Timer timer;
    GetSubproblemCut(subproblem_data_map);
    SetSubproblemCost(0);
    for (const auto& [_, subproblem_data]: subproblem_data_map)
    {
        SetSubproblemCost(GetSubproblemCost() + subproblem_data.subproblem_cost);
    }

    _data.subproblems_walltime = timer.elapsed();
    _data.ub = 0;
    BuildCutFull(subproblem_data_map);
}

/*!
 *  \brief Run BendersSequential algorithm
 *
 *  Method to run BendersSequential algorithm
 */
void BendersSequential::solve_master()
{
    get_master_value();
}

void BendersSequential::check_convergence()
{
    _data.stop = ShouldBendersStop();
}

Point BendersSequential::get_master_x() const
{
    return get_x_cut();
}

void BendersSequential::set_master_x(const Point& x)
{
    // In sequential, master_x is already in _data.x_cut after ComputeXCut
    // This is more relevant for MPI where rank != 0 needs to receive it.
}

#include "antares-xpansion/benders/benders_core/BendersAlgorithm.h"
#include "antares-xpansion/benders/benders_core/NoOuterLoopStrategy.h"
#include "antares-xpansion/benders/benders_core/StandardSubproblemSolver.h"
#include "antares-xpansion/benders/benders_sequential/SequentialCommunication.h"

void BendersSequential::launch()
{
    _logger->display_message("Building input");
    _logger->display_message("Constructing workers...");

    InitializeProblems();
    _logger->display_message("Running solver...");
    try
    {
        auto comm = std::make_shared<SequentialCommunication>();
        auto benders_ptr = std::shared_ptr<BendersBase>(this, [](BendersBase*) {});
        auto solver = std::make_shared<StandardSubproblemSolver>(benders_ptr);

        auto algorithm = std::make_shared<BendersAlgorithm>(comm, solver, nullptr, benders_ptr);
        auto no_outer_loop = std::make_shared<NoOuterLoopStrategy>([algorithm]()
                                                                   { algorithm->MasterLoop(); });
        algorithm->set_outer_loop(no_outer_loop);

        algorithm->Run();
        _logger->display_message(BendersName() + " solver terminated.");
    }
    catch (const std::exception& ex)
    {
        std::string error = "Exception raised : " + std::string(ex.what());
        _logger->display_message(error);
    }

    post_run_actions();
    free();
}

void BendersSequential::Run()
{
    // Delegate to launch() which handles the full sequential algorithm
    launch();
}
