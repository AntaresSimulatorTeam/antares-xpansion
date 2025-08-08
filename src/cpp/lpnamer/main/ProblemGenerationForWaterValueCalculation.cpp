
#include "antares-xpansion/lpnamer/main/ProblemGenerationForWaterValueCalculation.h"

#include <execution>
#include <iostream>
#include <utility>

#include <antares/api/solver.h>

#include "antares-xpansion/benders/output/OutputWriter.h"
#include "antares-xpansion/lpnamer/problem_modifier/XpansionProblemsFromAntaresProvider.h"
#include "malloc.h"

static const std::string LP_DIRNAME = "lp";

/// @brief Create the output directory and the lp directory if they do not exist
/// @param output_path The path to the output directory
static void CreateDirectories(const std::filesystem::path& output_path)
{
    if (!std::filesystem::exists(output_path))
    {
        std::filesystem::create_directories(output_path);
    }
    auto lp_path = output_path / LP_DIRNAME;
    if (!std::filesystem::exists(lp_path))
    {
        std::filesystem::create_directories(lp_path);
    }
}

/// @brief Constructor
/// @param options The options for the problem generation
/// @param problems The problems to be modified
/// @param gridDefinition The grid definition
ProblemGenerationForWaterValueCalculation::ProblemGenerationForWaterValueCalculation(
  ConfigurationManager::ConfigDirectories directories,
  std::string solverName):
    directories(directories)
{
    Antares::Solver::Optimization::OptimizationOptions optOptions;
    optOptions.firstOptimOptions.solverName = solverName;
    optOptions.secondOptimOptions.solverName = solverName;

    if (solverName == SolverConfig("xpress"))
    {
        optOptions.firstOptimOptions.solverParameters = "PRESOLVE 1";
        optOptions.secondOptimOptions.solverParameters = "PRESOLVE 1";
    }

    auto [results, error] = Antares::API::PerformSimulation(directories.study_dir,
                                                            directories.simulation_dir,
                                                            optOptions);

#ifndef _WIN32
    malloc_trim(0);
#endif

    // Handle errors
    if (error)
    {
        throw LogUtils::XpansionError<std::runtime_error>("Antares simulation failed:\n\t"
                                                            + error->reason,
                                                          LOGLOCATION);
    }

    XpansionProblemsFromAntaresProvider adapter(results);
    for (const auto& [pbId, _]: results.weeklyProblems)
    {
        auto solver_log_manager = SolverLogManager(directories.simulation_dir / "solver.log");
        auto problem = adapter.provideProblem("CBC", solver_log_manager, pbId);
        problems[pbId] = problem;
    }
}

/// @brief Update the problems for the water value calculation
/// @return The path to the output mps file
std::filesystem::path ProblemGenerationForWaterValueCalculation::updateProblems(
  const GridDefinition& gridDefinition)
{
    using namespace std::string_literals;

    const auto log_file_path = directories.simulation_dir / "lp"s / "ProblemGenerationLog.txt"s;

    CreateDirectories(directories.simulation_dir);
    auto logger = ProblemGenerationLog::BuildLogger(log_file_path,
                                                    std::cout,
                                                    "Problem Generation"s);

    auto outputMpsPath = CleanProblemsForBellmanCalculations(directories.simulation_dir,
                                                             log_file_path,
                                                             gridDefinition);

    return outputMpsPath;
}

/// @brief Clean the problems for the Bellman Values calculations
/// @param xpansion_output_dir The output directory
/// @param log_file_path The path to the log file
/// @return The path to the output mps file
std::filesystem::path
ProblemGenerationForWaterValueCalculation::CleanProblemsForBellmanCalculations(
  const std::filesystem::path& xpansion_output_dir,
  const std::filesystem::path& log_file_path,
  const GridDefinition& gridDefinition)
{
    auto solver_log_manager = SolverLogManager(log_file_path);

    // Create directory for Bellman problems
    auto outputMpsPath = xpansion_output_dir / ("mps_" + std::to_string(gridDefinition.gridID));
    std::filesystem::create_directory(outputMpsPath);
    std::for_each(std::execution::par,
                  problems.begin(),
                  problems.end(),
                  [&](auto& pb)
                  {
                      auto pbId = pb.first;
                      auto problem = std::make_shared<Problem>(pb.second->clone());
                      std::string pbName = "problem-" + std::to_string(pbId.year) + "-"
                                           + std::to_string(pbId.week) + "--optim-nb-1";
                      cleanProblemForBellmanCalculations(problem,
                                                         pbName,
                                                         gridDefinition,
                                                         pbId.week);

                      problem->write_prob_mps(outputMpsPath / (pbName + ".mps"));
                  });

    return outputMpsPath;
}

/// @brief Clean the problem for the Bellman Values calculations
/// @param problem The problem to clean
/// @param gridDefinition The grid definition
/// @param week The week to clean
void cleanProblemForBellmanCalculations(std::shared_ptr<Problem> problem,
                                        std::string& pbName,
                                        const GridDefinition& gridDefinition,
                                        int week)
{
    for (const auto& gridElement: gridDefinition.gridElements)
    {
        if (gridElement.problemName == "all" || gridElement.problemName == pbName)
        {
            for (int hour = (week - 1) * 168; hour < week * 168; ++hour)
            {
                // Delete variables HydroLevel and Overflow
                int idx = problem->get_col_index("HydroLevel::area<" + gridElement.area + ">::hour<"
                                                 + std::to_string(hour) + ">");
                problem->del_cols(idx, idx);

                idx = problem->get_col_index("Overflow::area<" + gridElement.area + ">::hour<"
                                             + std::to_string(hour) + ">");
                problem->del_cols(idx, idx);

                // Delete constraints AreaHydroLevel
                idx = problem->get_row_index("AreaHydroLevel::area<" + gridElement.area + ">::hour<"
                                             + std::to_string(hour) + ">");
                problem->del_rows(idx, idx);

                // Reset HydroProd as it might have been modified by heuristic
                idx = problem->get_col_index("HydProd::area<" + gridElement.area + ">::hour<"
                                             + std::to_string(hour) + ">");
                problem->chg_bounds(
                  {idx},
                  {'U'},
                  {gridDefinition.reservoirs.at(gridElement.area).max_generating[week - 1]
                   / Reservoir::hours_in_week});
            }
        }
    }
}
