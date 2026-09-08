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
                std::make_shared<SequentialCommunicationStrategy>()),
    cuts_manager_(_data, _problem_to_id, relevantIterationData_, _master)
{
}

void BendersSequential::InitializeProblems()
{
    MatchProblemToId();

    std::vector<SubProblemNamesInCut> subproblem_per_cut_indices;

    // Skip cut aggregation when there are no subproblems to avoid division by zero
    if (_data.nsubproblem > 0) [[likely]]
    {
        int n_cuts = SetAggregation(_data.nsubproblem);
        subproblem_per_cut_indices.reserve(n_cuts);

        SubProblemNamesInCut current_cut;
        size_t group_size = (_data.nsubproblem + n_cuts - 1) / n_cuts;
        current_cut.reserve(group_size);

        for (const auto& [name, id]: _problem_to_id)
        {
            current_cut.emplace_back(name, 0);
            if (current_cut.size() == group_size)
            {
                subproblem_per_cut_indices.emplace_back(std::move(current_cut));
                current_cut.clear();
                current_cut.reserve(group_size);
            }
        }
        if (!current_cut.empty())
        {
            subproblem_per_cut_indices.emplace_back(std::move(current_cut));
        }
    }
    cuts_manager_.SetSubproblemPerCutIndices(std::move(subproblem_per_cut_indices));

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
                               GetSubCutTolerance());
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
    GetSubproblemCut(subproblem_data_map, MakeFastBeginHook(), MakeCacheBeginHook(), nullptr);
    SetSubproblemCost(0);
    for (const auto& [_, subproblem_data]: subproblem_data_map)
    {
        SetSubproblemCost(GetSubproblemCost() + subproblem_data.subproblem_cost);
    }

    _data.subproblems_walltime = timer.elapsed();
    check_status(subproblem_data_map);
    cuts_manager_.GatherAndBuildCuts(subproblem_data_map);
}

/*!
 *  \brief Run BendersSequential algorithm
 *
 *  Method to run BendersSequential algorithm
 */
void BendersSequential::Run()
{
    init_data();
    ChecksResumeMode();
    if (is_trace())
    {
        OpenCsvFile();
    }

    HandleInitialMasterRelaxation();

    while (!_data.stop)
    {
        Timer timer_master;
        ++_data.it;

        if (SwitchToIntegerMaster(_data.is_in_initial_relaxation))
        {
            _logger->LogAtSwitchToInteger();
            ActivateIntegrityConstraints();
            ResetDataPostRelaxation();
        }

        _logger->log_at_initialization(_data.it + GetNumIterationsBeforeRestart());
        _logger->display_message("\tSolving master...");
        get_master_value();
        _logger->log_master_solving_duration(_data.timer_master);

        cuts_manager_.ComputeXCut(_data,
                                  Options().SEPARATION_PARAM,
                                  Options().MASTER_SOLUTION_TOLERANCE);
        _logger->log_iteration_candidates(bendersDataToLogData(_data));

        _logger->display_message("\tSolving subproblems...");
        BuildCut();
        _logger->LogSubproblemsSolvingWalltime(_data.subproblems_walltime);

        compute_ub();
        update_best_ub();

        _logger->log_at_iteration_end(bendersDataToLogData(_data));

        UpdateTrace();

        _data.timer_master = timer_master.elapsed();
        _data.iteration_time = -_data.benders_time;
        _data.benders_time = GetBendersTime();
        _data.iteration_time += _data.benders_time;
        _data.stop = ShouldBendersStop();
        SaveCurrentBendersData();
    }
    CloseCsvFile();
    EndWritingInOutputFile();
    write_basis();
}

void BendersSequential::launch()
{
    _logger->display_message("Building input");
    _logger->display_message("Constructing workers...");

    InitializeProblems();
    _logger->display_message("Running solver...");
    try
    {
        Run();
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
