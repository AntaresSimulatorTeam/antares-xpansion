
#include "antares-xpansion/lpnamer/main/ProblemGenerationForWaterValueCalculation.h"

#include <execution>
#include <iostream>
#include <tbb/parallel_for_each.h>
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

    for (auto& gridDefinition: grid.gridDefinitions | std::views::values)
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
        return WaterValueComputationMode::SEQUENTIAL_IGNORE_TRAJECTORY;
    }
    return WaterValueComputationMode::SEQUENTIAL_UPDATE_TRAJECTORY;
}

/// @brief Launch the simulation and save the problems satisfying startWeek <= week <= endweek
ProblemGenerationForWaterValueCalculation::ProblemGenerationForWaterValueCalculation(
  ConfigurationManager::ConfigDirectories directories,
  Logger logger,
  const std::string& solverName,
  unsigned int startWeek,
  unsigned int endWeek,
  bool writePbFiles,
  const std::string& problemFormat,
  const WaterValueComputationMode& computationMode):
    directories(directories),
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
/// @param gridDefinition
/// @return The modified problems
std::map<Antares::Solver::WeeklyProblemId, std::shared_ptr<Problem>>
ProblemGenerationForWaterValueCalculation::updateProblems(
  const GridDefinition& gridDefinition,
  const ReservoirManagement& reservoirManagement,
  const std::string& areaName)
{
    using namespace std::string_literals;

    const auto log_file_path = directories.simulation_dir / "lp"s / "ProblemGenerationLog.txt"s;

    CreateDirectories(directories.simulation_dir);

    logger->display_message("Updating problems");
    logger->display_message("Reservoir area: '" + reservoirManagement.reservoir.area + "'");
    logger->display_message("areaName: " + areaName);
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
/// @param gridDefinition The gridDefinition
/// @return The modified problems
std::map<Antares::Solver::WeeklyProblemId, std::shared_ptr<Problem>>
ProblemGenerationForWaterValueCalculation::cleanProblemsForBellmanCalculations(
  const std::filesystem::path& xpansion_output_dir,
  const std::filesystem::path& log_file_path,
  const GridDefinition& gridDefinition,
  const ReservoirManagement& reservoirManagement,
  const std::string& areaName)
{
    logger->display_message("Cleaning problems for Bellman calculations");
    std::map<Antares::Solver::WeeklyProblemId, std::shared_ptr<Problem>> modifiedProblems;

    // Create directory for Bellman problems
    auto outputMpsPath = xpansion_output_dir / ("mps_" + std::to_string(gridDefinition.gridID));
    std::filesystem::create_directory(outputMpsPath);
    tbb::parallel_for_each(
      problems.begin(),
      problems.end(),
      [&](auto& pb)
      {
          auto pbId = pb.first;
          if (startWeek <= pbId.week && pbId.week <= endWeek)
          {
              // copy of the problem needed if gridCollection contains multiple
              // gridDefinitions, and for multistock
              std::shared_ptr<Problem> problem = std::make_shared<Problem>(*(pb.second->clone()));
              std::string pbName = "problem-" + std::to_string(pbId.year) + "-"
                                   + std::to_string(pbId.week) + "--optim-nb-1";
              //   logger->display_message("Modifying problem: " + pbName);
              cleanProblemForBellmanCalculations(problem,
                                                 gridDefinition,
                                                 reservoirManagement,
                                                 areaName,
                                                 pbName,
                                                 pbId);
              //   logger->display_message("Problem: " + pbName + " modified");
              modifiedProblems[pbId] = problem;

              if (writePbFiles)
              {
                  logger->display_message("Writing problem " + pbName + " to disk...");
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

template<int (Problem::*Getter)(const std::string&)>
int checked(Problem* p, const std::string& name, const Antares::Solver::WeeklyProblemId& pbid)
{
    int idx = (p->*Getter)(name);
    if (idx == -1)
    {
        throw std::runtime_error("Index not found: " + name + " for scenario "
                                 + std::to_string(pbid.year) + " and year "
                                 + std::to_string(pbid.week));
    }
    return idx;
}

/// @brief Clean the problem for the Bellman Values calculations
/// @param problem The problem to clean
/// @param pnName The problem name
/// @param gridDefinition The grid definition
/// @param pbID The problem ID (year/week)
void ProblemGenerationForWaterValueCalculation::cleanProblemForBellmanCalculations(
  std::shared_ptr<Problem> problem,
  const GridDefinition& gridDefinition,
  const ReservoirManagement& reservoirManagement,
  const std::string& areaName,
  std::string& pbName,
  Antares::Solver::WeeklyProblemId pbID)
{
    for (const auto& gridElement: gridDefinition.gridElements)
    {
        logger->display_message("gridElement: " + gridElement.area);
        logger->display_message("reservoir area: " + reservoirManagement.reservoir.area);

        if (gridElement.problemName == "all" || gridElement.problemName == pbName)
        {
            // default multivariate case: cleaning all gridElements
            if (this->computationMode
                == ProblemGenerationForWaterValueCalculation::WaterValueComputationMode::
                  MULTIVARIATE)
            {
                logger->display_message("cleanReservoirConstraints in multivariate mode");
                logger->display_message("reservoir: " + reservoirManagement.reservoir.area);
                cleanReservoirConstraints(problem, reservoirManagement.reservoir, pbID);
            }
            // targetting a specific stock in a multistock use case (with or without trajectory)
            else if (areaName == gridElement.area)
            {
                logger->display_message("cleanReservoirConstraints in multistock mode");
                cleanReservoirConstraints(problem,
                                          gridDefinition.reservoirs.at(gridElement.area),
                                          pbID);
            }
        }
    }

    // other gridElements in a multistock context must be updated with their optimal
    // trajectories
    if (gridDefinition.gridElements.size() == 1
        && this->computationMode
             != ProblemGenerationForWaterValueCalculation::WaterValueComputationMode::MULTIVARIATE)
    {
        for (auto& reservoir: gridDefinition.reservoirs)
        {
            if (reservoir.second.area != areaName)
            {
                if (gridDefinition.gridElements[0].problemName == "all"
                    || gridDefinition.gridElements[0].problemName == pbName)
                {
                    // if SEQUENTIAL_IGNORE_TRAJECTORY: no update of the optimal trajectory
                    if (computationMode == WaterValueComputationMode::SEQUENTIAL_UPDATE_TRAJECTORY)
                    {
                        // logger->display_message(
                        //   "Other gridElement in a multistock context updated with its trajectory:
                        //   "
                        //   + reservoir.second.area);
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
  Antares::Solver::WeeklyProblemId pbID)
{
    for (int hour = (pbID.week - 1) * 168; hour < pbID.week * 168; ++hour)
    {
        // ==== DELETE HydroLevel ====
        {
            std::string name = "HydroLevel::area<" + reservoir.area + ">::hour<"
                               + std::to_string(hour) + ">";
            int idx = problem->get_col_index(name);
            // int idx = checked<&Problem::get_col_index>(problem.get(), name, pbID);
            problem->del_cols(idx, idx);
        }

        // ==== DELETE Overflow ====
        {
            std::string name = "Overflow::area<" + reservoir.area + ">::hour<"
                               + std::to_string(hour) + ">";
            int idx = checked<&Problem::get_col_index>(problem.get(), name, pbID);
            problem->del_cols(idx, idx);
        }

        // ==== DELETE AreaHydroLevel constraint ====
        {
            std::string name = "AreaHydroLevel::area<" + reservoir.area + ">::hour<"
                               + std::to_string(hour) + ">";
            int idx = checked<&Problem::get_row_index>(problem.get(), name, pbID);
            problem->del_rows(idx, idx);
        }

        // ==== RESET HydroProd bounds ====
        {
            std::string name = "HydProd::area<" + reservoir.area + ">::hour<" + std::to_string(hour)
                               + ">";
            int idx = checked<&Problem::get_col_index>(problem.get(), name, pbID);

            problem->chg_bounds({idx},
                                {'U'},
                                {reservoir.max_generating[pbID.week - 1]
                                 / Reservoir::hours_in_week});
        }
    }
}

void ProblemGenerationForWaterValueCalculation::updateReservoirWithOptimalTrajectory(
  std::shared_ptr<Problem> problem,
  const Reservoir& reservoir,
  Antares::Solver::WeeklyProblemId pbID)
{
    // Updating constraint with optimal trajectory
    logger->display_message("Updating constraint with optimal trajectory: week "
                            + std::to_string(pbID.week));

    // logger->display_message("Optimal trajectory size: "
    //                         + std::to_string(reservoir.optimal_trajectory.size()));
    double optimalTrajectoryValue = -reservoir.optimal_trajectory[pbID.week][pbID.year - 1]
                                    + reservoir.optimal_trajectory[pbID.week - 1][pbID.year - 1]
                                    + reservoir.inflow[pbID.week - 1][pbID.year - 1];
    // logger->display_message("Optimal trajectory value: " +
    // std::to_string(optimalTrajectoryValue));
    problem->fix_rhs_to("HydroPower::area<" + reservoir.area + ">::week<"
                          + std::to_string(pbID.week - 1) + ">",
                        optimalTrajectoryValue);
}

void ProblemGenerationForWaterValueCalculation::initializeOptimalTrajectories(
  std::shared_ptr<GridCollection> gridCollection) const
{
    for (auto& reservoir: gridCollection->reservoirs | std::views::values)
    {
        logger->display_message("Initializing optimal trajectory for reservoir " + reservoir.area);
        reservoir.initializeOptimalTrajectory(startWeek, endWeek);
        logger->display_message("Reservoir " + reservoir.area + " has been initialized with "
                                + std::to_string(reservoir.optimal_trajectory.size()) + " by "
                                + std::to_string(reservoir.optimal_trajectory[0].size())
                                + " elements");
    }
}
