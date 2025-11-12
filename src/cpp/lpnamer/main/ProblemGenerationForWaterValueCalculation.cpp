
#include "antares-xpansion/lpnamer/main/ProblemGenerationForWaterValueCalculation.h"

#include <execution>
#include <iostream>
#include <utility>

#include <antares/api/solver.h>

#include "antares-xpansion/benders/output/OutputWriter.h"
#include "antares-xpansion/helpers/solver_utils.h"
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

/// @brief Static function determining the computation mode for water values. Can be multivariate,
/// multistock, sequential
/// @param grid the GridCollection based on which the mode must be determined
/// @param ignoreOptimalTrajectory a boolean to ignore optimal trajectories in a multistock context,
/// falling back on sequential (default is false)
/// @return
ProblemGenerationForWaterValueCalculation::WaterValueComputationMode
ProblemGenerationForWaterValueCalculation::getComputationModeFromGrid(const GridCollection& grid,
                                                                      bool ignoreOptimalTrajectory)
{
    if (grid.gridDefinitions.size() == 1)
    {
        // one single gridID, no matter the number of elements
        return WaterValueComputationMode::MULTIVARIATE;
    }

    for (auto& gridDefinition: grid.gridDefinitions)
    {
        if (gridDefinition.gridElements.size() > 1)
        {
            // error: several gridIDs with several gridElements
            throw std::domain_error(
              "ERROR: grid.csv has multiple grid IDs, and multiple elements in at least one of "
              "them. This use case is not supported.");
        }
    }
    if (ignoreOptimalTrajectory)
    {
        return WaterValueComputationMode::SEQUENTIAL;
    }
    return WaterValueComputationMode::MULTISTOCK;
}

/// @brief Constructor
/// @param options The options for the problem generation
/// @param problems The problems to be modified
/// @param gridDefinition The grid definition
ProblemGenerationForWaterValueCalculation::ProblemGenerationForWaterValueCalculation(
  ConfigurationManager::ConfigDirectories directories,
  //   const ReservoirManagement& reservoirManagement,
  Logger logger,
  const std::string& solverName,
  unsigned int startWeek,
  unsigned int endWeek,
  bool writePbFiles,
  const std::string& problemFormat,
  const WaterValueComputationMode& computationMode):
    directories(directories),
    // reservoirManagement(reservoirManagement),
    logger(std::move(logger)),
    startWeek(startWeek),
    endWeek(endWeek),
    writePbFiles(writePbFiles),
    problemFormat(problemsFormatFromString(problemFormat)),
    computationMode(computationMode)
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
        auto problem = adapter.provideProblem(solverName == SolverConfig("xpress") ? "xpress"
                                                                                   : "CBC",
                                              solver_log_manager,
                                              pbId);
        problems[pbId] = problem;
    }

    if (!problems.empty())
    {
        this->startWeek = std::max(startWeek, problems.begin()->first.week);
        this->endWeek = std::min(endWeek, problems.rbegin()->first.week);
    }
}

/// @brief Update the problems for the water value calculation
/// @return The modified problems
std::map<Antares::Solver::WeeklyProblemId, std::shared_ptr<Problem>>
ProblemGenerationForWaterValueCalculation::updateProblems(
  const GridDefinition& gridDefinition,
  const ReservoirManagement& reservoirManagement,
  const std::optional<std::string>& areaName)
{
    using namespace std::string_literals;

    const auto log_file_path = directories.simulation_dir / "lp"s / "ProblemGenerationLog.txt"s;

    CreateDirectories(directories.simulation_dir);
    auto logger = ProblemGenerationLog::BuildLogger(log_file_path,
                                                    std::cout,
                                                    "Problem Generation"s);

    logger->display_message("Updating problems");
    auto modifiedProblems = cleanProblemsForBellmanCalculations(directories.simulation_dir,
                                                                log_file_path,
                                                                gridDefinition,
                                                                reservoirManagement,
                                                                areaName);

    return modifiedProblems;
}

/// @brief Clean the problems for the Bellman Values calculations
/// @param xpansion_output_dir The output directory
/// @param log_file_path The path to the log file
/// @return The modified problems
std::map<Antares::Solver::WeeklyProblemId, std::shared_ptr<Problem>>
ProblemGenerationForWaterValueCalculation::cleanProblemsForBellmanCalculations(
  const std::filesystem::path& xpansion_output_dir,
  const std::filesystem::path& log_file_path,
  const GridDefinition& gridDefinition,
  const ReservoirManagement& reservoirManagement,
  const std::optional<std::string>& areaName)
{
    auto solver_log_manager = SolverLogManager(log_file_path);
    std::map<Antares::Solver::WeeklyProblemId, std::shared_ptr<Problem>> modifiedProblems;

    // Create directory for Bellman problems
    auto outputMpsPath = xpansion_output_dir / ("mps_" + std::to_string(gridDefinition.gridID));
    std::filesystem::create_directory(outputMpsPath);
    std::for_each(std::execution::par,
                  problems.begin(),
                  problems.end(),
                  [&](auto& pb)
                  {
                      auto pbId = pb.first;
                      if (startWeek <= pbId.week && pbId.week <= endWeek)
                      {
                          // copy of the problem needed if gridCollection contains multiple
                          // gridDefinitions, and for multistock
                          std::shared_ptr<Problem> problem = std::make_shared<Problem>(
                            SolverFactory::copy_solver(*(pb.second)));

                          std::string pbName = "problem-" + std::to_string(pbId.year) + "-"
                                               + std::to_string(pbId.week) + "--optim-nb-1";

                          logger->display_message(
                            "cleanProblemForBellmanCalculations... for area '" + areaName.value()
                            + "' for week " + std::to_string(pbId.week) + " of "
                            + std::to_string(endWeek) + " of year " + std::to_string(pbId.year));
                          cleanProblemForBellmanCalculations(problem,
                                                             gridDefinition,
                                                             reservoirManagement,
                                                             areaName,
                                                             pbName,
                                                             pbId);
                          logger->display_message("cleanProblemForBellmanCalculations OK");
                          modifiedProblems[pbId] = problem;

                          if (writePbFiles)
                          {
                              switch (problemFormat)
                              {
                              case ProblemsFormat::MPS_FILE:
                                  problem->write_prob_mps(outputMpsPath / (pbName + ".mps"));
                                  break;
                              case ProblemsFormat::OPTIMIZED:
                                  problem->save_prob(outputMpsPath / (pbName + ".svf"));
                                  break;
                                  // potential errors are handled by
                                  // problemsFormatFromString in constructor
                              }
                          }
                      }
                  });

    return modifiedProblems;
}

/// @brief Clean the problem for the Bellman Values calculations
/// @param problem The problem to clean
/// @param gridDefinition The grid definition
/// @param week The week to clean
void ProblemGenerationForWaterValueCalculation::cleanProblemForBellmanCalculations(
  std::shared_ptr<Problem> problem,
  const GridDefinition& gridDefinition,
  const ReservoirManagement& reservoirManagement,
  const std::optional<std::string>& areaName,
  std::string& pbName,
  Antares::Solver::WeeklyProblemId pbID)
{
    for (const auto& gridElement: gridDefinition.gridElements)
    {
        logger->display_message("gridElement: " + gridElement.area);
        logger->display_message("areaName: " + areaName.value());
        if (areaName == std::nullopt /* default multivariate case: cleaning all gridElements */
            || areaName.value()
                 == gridElement.area /* targetting a specific stock in a multistock use case*/)
        {
            if (gridElement.problemName == "all" || gridElement.problemName == pbName)
            {
                logger->display_message("cleanReservoirConstraints");
                cleanReservoirConstraints(problem,
                                          gridDefinition.reservoirs.at(gridElement.area),
                                          pbID);
            }
        }
    }

    // other gridElements in a multistock context must be updated with their optimal
    // trajectories
    if (gridDefinition.gridElements.size() == 1 && areaName != std::nullopt)
    {
        for (auto& reservoir: gridDefinition.reservoirs)
        {
            if (reservoir.second.area != areaName)
            {
                if (gridDefinition.gridElements[0].problemName == "all"
                    || gridDefinition.gridElements[0].problemName == pbName)
                {
                    // if SEQUENTIAL: no update of the optimal trajectory
                    if (computationMode == WaterValueComputationMode::MULTISTOCK)
                    {
                        logger->display_message(
                          "Other gridElement in a multistock context updated with its trajectory: "
                          + reservoir.second.area);
                        cleanReservoirConstraints(problem, reservoir.second, pbID);
                        updateReservoirWithOptimalTrajectory(problem, reservoir.second, pbID);
                    }
                    else
                    {
                        logger->display_message("No update of the optimal trajectory for area: "
                                                + reservoir.second.area);
                    }
                }
            }
        }
    }
}

void ProblemGenerationForWaterValueCalculation::cleanReservoirConstraints(
  std::shared_ptr<Problem> problem,
  const Reservoir& reservoir,
  Antares::Solver::WeeklyProblemId pbId)
{
    for (int hour = (pbId.week - 1) * 168; hour < pbId.week * 168; ++hour)
    {
        // logger->display_message("hour: " + std::to_string(hour));

        // Delete variables HydroLevel and Overflow
        // logger->display_message("Delete variables HydroLevel and Overflow");
        int idx = problem->get_col_index("HydroLevel::area<" + reservoir.area + ">::hour<"
                                         + std::to_string(hour) + ">");
        problem->del_cols(idx, idx);

        idx = problem->get_col_index("Overflow::area<" + reservoir.area + ">::hour<"
                                     + std::to_string(hour) + ">");
        problem->del_cols(idx, idx);

        // Delete constraints AreaHydroLevel
        // logger->display_message("Delete constraints AreaHydroLevel");
        idx = problem->get_row_index("AreaHydroLevel::area<" + reservoir.area + ">::hour<"
                                     + std::to_string(hour) + ">");
        problem->del_rows(idx, idx);

        // Reset HydroProd as it might have been modified by heuristic
        // logger->display_message("Reset HydroProd as it might have been modified by heuristic");
        idx = problem->get_col_index("HydProd::area<" + reservoir.area + ">::hour<"
                                     + std::to_string(hour) + ">");
        problem->chg_bounds(
          {idx},
          {'U'},
          //   {gridDefinition.reservoirs.at(gridElement.area).max_generating[pbID.week - 1]
          //    / Reservoir::hours_in_week});
          {reservoir.max_generating[pbId.week - 1] / Reservoir::hours_in_week});
    }
}

void ProblemGenerationForWaterValueCalculation::updateReservoirWithOptimalTrajectory(
  std::shared_ptr<Problem> problem,
  const Reservoir& reservoir,
  Antares::Solver::WeeklyProblemId pbId)
{
    for (int hour = (pbId.week - 1) * 168; hour < pbId.week * 168; ++hour)
    {
        // Updating constraint with optimal trajectory
        // logger->display_message("Updating constraint with optimal trajectory: hour "
        //                         + std::to_string(hour) + " and week " +
        //                         std::to_string(pbId.week));
        int idx = problem->get_row_index("HydroPower::area<" + reservoir.area + ">::week<"
                                         + std::to_string(pbId.week - 1) + ">");
        // logger->display_message("optimal trajectory size: "
        //                         + std::to_string(reservoir.optimal_trajectory.size()));
        // logger->display_message(
        //   "optimal trajectory value: "
        //   + std::to_string(reservoir.optimal_trajectory[pbId.week - 1][pbId.year - 1]));
        problem->chg_bounds({idx},
                            {'U'},
                            {reservoir.optimal_trajectory[pbId.week - 1][pbId.year - 1]});
    }
}

void ProblemGenerationForWaterValueCalculation::initializeOptimalTrajectories(
  std::shared_ptr<GridCollection> gridCollection) const
{
    for (auto& reservoir: gridCollection->reservoirs)
    {
        reservoir.second.optimal_trajectory = std::vector<std::vector<double>>(
          reservoir.second.inflow.begin() + startWeek - 1,
          reservoir.second.inflow.begin() + endWeek);
    }
}
