
#include "antares-xpansion/benders/grid_search/GridSearch.h"

#include <utility>

#include "antares-xpansion/benders/benders_core/CriterionComputation.h"
#include "antares-xpansion/benders/benders_core/CustomVector.h"
#include "antares-xpansion/helpers/Timer.h"

GridSearch::GridSearch(const BendersBaseOptions& options,
                       Logger logger,
                       std::shared_ptr<Output::OutputWriter> writer,
                       std::shared_ptr<MathLoggerDriver> mathLoggerDriver):
    BendersBase(options, std::move(logger), std::move(writer), std::move(mathLoggerDriver))
{
}

/*!
 *  \brief Method to load each problem in a thread
 *
 *  The initialization of each problem is done sequentially
 *
 */

void GridSearch::InitializeProblems()
{
    MatchProblemToId();

    BuildMasterProblem();
    int current_problem_id = 0;
    // Dispatch subproblems to process
    for (const auto& problem: coupling_map_)
    {
        const auto subProblemFilePath = GetSubproblemPath(problem.first);
        AddSubproblem(problem);
        AddSubproblemName(problem.first);
        current_problem_id++;
    }
    init_problems_ = false;
}

void GridSearch::BuildMasterProblem()
{
    reset_master<WorkerMaster>(master_variable_map_,
                               get_master_path(),
                               get_solver_name(),
                               get_log_level(),
                               _data.nsubproblem,
                               solver_log_manager_,
                               IsResumeMode(),
                               _logger,
                               Options().PROBLEMS_FORMAT);
}

void GridSearch::SolveSubproblem(PlainData::SubProblemData& subproblem_data,
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

/*!
 *  \brief Method to free the memory used by each problem
 */
void GridSearch::free()
{
    free_subproblems();
}

/*!
 *  \brief Run Benders algorithm in parallel
 *
 *  Method to run Benders algorithm in parallel
 *
 */
void GridSearch::Run()
{
    // Fill the grid points vector with file
    // Example :
    // subproblem1 variable1 value1 variable2 value2
    // subproblem1 variable1 value3 variable2 value4
    // ...
    std::ifstream grid_file("/home/workspace/antares-xpansion-generic/data_test/examples/"
                            "xpansion-test-02-new/user/expansion/grid_points.txt");
    std::string line;
    while (std::getline(grid_file, line))
    {
        // Fill the map of vector of points with the line content and the subproblem name
        std::istringstream iss(line);
        std::string subproblem_name;
        iss >> subproblem_name;
        Point point;
        std::string variable;
        double value;
        while (iss >> variable >> value)
        {
            point[variable] = value;
        }
        gridPoints[subproblem_name].push_back(point);
    }

    // For each subproblem, solve the subproblem with fixed candidates for each point in the grid
    std::map<std::string, std::pair<Point, double>> optimalSolutions;

    for (const auto& [subproblem_name, points]: gridPoints)
    {
        double minCost = std::numeric_limits<double>::max();
        Point optimalPoint;

        for (const auto& point: points)
        {
            for (const auto& [variable, value]: point)
            {
                _data.x_cut[variable] = value;
            }

            if (subproblem_map.find(subproblem_name) == subproblem_map.end())
            {
                throw std::runtime_error("Subproblem " + subproblem_name + " not found in subproblem map");
            }
            auto& worker = subproblem_map[subproblem_name];
            PlainData::SubProblemData subproblem_data;
            SolveSubproblem(subproblem_data, subproblem_name, worker);

            // Print the subProblem cost for the point
            std::cout << "Subproblem: " << subproblem_name
                      << " Cost: " << subproblem_data.subproblem_cost << std::endl;

            // Track the optimal solution
            if (subproblem_data.subproblem_cost < minCost)
            {
                minCost = subproblem_data.subproblem_cost;
                optimalPoint = point;
            }
        }

        // Store the optimal solution for the subproblem
        optimalSolutions[subproblem_name] = {optimalPoint, minCost};
    }

    // Print the optimal solutions
    for (const auto& [subproblem_name, solution]: optimalSolutions)
    {
        std::cout << "Optimal solution for subproblem: " << subproblem_name
                  << " with cost: " << solution.second << std::endl;
        for (const auto& [variable, value]: solution.first)
        {
            std::cout << "  " << variable << ": " << value << std::endl;
        }
    }
}

void GridSearch::PreRunInitialization()
{
    init_data();

    if (is_initial_relaxation_requested())
    {
        _logger->LogAtInitialRelaxation();
        DeactivateIntegrityConstraints();
        SetDataPreRelaxation();
    }

    mathLoggerDriver_->write_header();
    init_data_ = false;
}

void GridSearch::launch()
{
    std::cout << "Launching grid search" << std::endl;

    InitializeProblems();

    Run();
}
