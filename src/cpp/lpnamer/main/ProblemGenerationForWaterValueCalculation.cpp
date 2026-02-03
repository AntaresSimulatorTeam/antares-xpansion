
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
ProblemGenerationForWaterValueCalculation::getComputationModeFromGrid(bool ignoreOptimalTrajectory)
{
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
  const WaterValueComputationMode& computationMode,
  unsigned int startWeek,
  unsigned int endWeek,
  bool writePbFiles,
  const std::string& problemFormat):
    directories(directories),
    logger(std::move(logger)),
    computationMode(computationMode),
    startWeek(startWeek),
    endWeek(endWeek),
    writePbFiles(writePbFiles),
    problemFormat(problemsFormatFromString(problemFormat))
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
    // auto solver_log_manager = SolverLogManager(directories.simulation_dir / "solver.log");
    // problems opening too many log files, and not writing anything yet.
    // for now: no log files passed to the problems.
    auto solver_log_manager = SolverLogManager();
    for (const auto& [pbId, _]: results.weeklyProblems)
    {
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
  const std::optional<std::string>& areaName)
{
    using namespace std::string_literals;

    const auto log_file_path = directories.simulation_dir / "lp"s / "ProblemGenerationLog.txt"s;

    CreateDirectories(directories.simulation_dir);

    logger->display_message("Updating problems");
    // logger->display_message("Reservoir area: '" + reservoirManagement.reservoir.area + "'");
    logger->display_message("areaName: " + areaName.value_or(""));
    // check added instead of passing the entire reservoirManagement, in a multistock context
    if (areaName == std::nullopt)
    {
        logger->display_message("The areaName for the current reservoir must be provided in the "
                                "context of a multistock computation. First element is assumed: "
                                + gridDefinition.gridElements[0].area);
    }
    auto modifiedProblems = cleanProblemsForBellmanCalculations(
      directories.simulation_dir,
      log_file_path,
      gridDefinition,
      areaName.value_or(gridDefinition.gridElements[0].area));

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
              cleanProblemForBellmanCalculations(problem, gridDefinition, areaName, pbName, pbId);
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

int checkedMapLookup(const std::unordered_map<std::string, int>& nameToIndex,
                     const std::string& name,
                     const Antares::Solver::WeeklyProblemId& pbID)
{
    auto it = nameToIndex.find(name);
    if (it == nameToIndex.end())
    {
        throw std::runtime_error("Index not found: " + name + " for scenario "
                                 + std::to_string(pbID.year) + " and week "
                                 + std::to_string(pbID.week));
    }
    return it->second;
}

inline std::string trimTrailingSpaces(const std::string& str)
{
    size_t end = str.find_last_not_of(' ');
    return (end == std::string::npos) ? "" : str.substr(0, end + 1);
}

/// @brief Clean the problem for the Bellman Values calculations
/// @param problem The problem to clean
/// @param pbName The problem name
/// @param gridDefinition The grid definition
/// @param pbID The problem ID (year/week)
void ProblemGenerationForWaterValueCalculation::cleanProblemForBellmanCalculations(
  std::shared_ptr<Problem> problem,
  const GridDefinition& gridDefinition,
  const std::string& areaName,
  std::string& pbName,
  Antares::Solver::WeeklyProblemId pbID)
{
    // Build index maps once
    int ncols = problem->get_ncols();
    int nrows = problem->get_nrows();

    std::vector<std::string> colNames = problem->get_col_names(0, ncols - 1);
    std::vector<std::string> rowNames = problem->get_row_names(0, nrows - 1);

    // Collect indices to delete and bounds to change
    AffectedColsAndRows affectedColsAndRows;

    for (int i = 0; i < ncols; ++i)
    {
        affectedColsAndRows.colNameToIndex[trimTrailingSpaces(colNames[i])] = i;
    }

    for (int i = 0; i < nrows; ++i)
    {
        affectedColsAndRows.rowNameToIndex[trimTrailingSpaces(rowNames[i])] = i;
    }

    int weekStart = (pbID.week - 1) * 168;

    int weekEnd = pbID.week * 168;

    for (const auto& gridElement: gridDefinition.gridElements)
    {
        logger->display_message("gridElement: " + gridElement.area);

        if (gridElement.problemName == "all" || gridElement.problemName == pbName)
        {
            // it was checked earlier that there is only one area in gridDefinition
            cleanReservoirConstraints(problem,
                                      gridDefinition.reservoirs.at(gridElement.area),
                                      pbID,
                                      affectedColsAndRows);
        }
    }

    // other gridElements/reservoirs in a multistock context must be updated with their optimal
    // trajectories, for this specific flag
    if (gridDefinition.gridElements.size() == 1
        && this->computationMode == WaterValueComputationMode::SEQUENTIAL_UPDATE_TRAJECTORY
        && (gridDefinition.gridElements[0].problemName == "all"
            || gridDefinition.gridElements[0].problemName == pbName))
    {
        for (auto& reservoir: gridDefinition.reservoirs)
        {
            if (reservoir.second.area != areaName)
            {
                cleanReservoirConstraints(problem, reservoir.second, pbID, affectedColsAndRows);
                updateReservoirWithOptimalTrajectory(problem, reservoir.second, pbID);
            }
        }
    }
    // Sort in descending order to preserve indices during deletion
    std::sort(affectedColsAndRows.colsToDelete.rbegin(), affectedColsAndRows.colsToDelete.rend());
    std::sort(affectedColsAndRows.rowsToDelete.rbegin(), affectedColsAndRows.rowsToDelete.rend());

    // Batch change bounds (must be done before deletions to preserve indices)
    if (!affectedColsAndRows.hydroProdCols.empty())
    {
        problem->chg_bounds(affectedColsAndRows.hydroProdCols,
                            std::vector<char>(affectedColsAndRows.hydroProdCols.size(), 'U'),
                            affectedColsAndRows.hydroProdBounds);
    }

    for (int idx: affectedColsAndRows.colsToDelete)
    {
        problem->del_cols(idx, idx);
    }

    for (int idx: affectedColsAndRows.rowsToDelete)
    {
        problem->del_rows(idx, idx);
    }
}

void ProblemGenerationForWaterValueCalculation::cleanReservoirConstraints(
  std::shared_ptr<Problem> problem,
  const Reservoir& reservoir,
  Antares::Solver::WeeklyProblemId pbID,
  AffectedColsAndRows& affectedColsAndRows)
{
    double maxGen = reservoir.max_generating[pbID.week - 1] / Reservoir::hours_in_week;
    const std::string& area = reservoir.area;
    for (int hour = (pbID.week - 1) * 168; hour < pbID.week * 168; ++hour)
    {
        std::string hourStr = std::to_string(hour);
        // ==== DELETE HydroLevel ===
        {
            std::string name = "HydroLevel::area<" + reservoir.area + ">::hour<"
                               + std::to_string(hour) + ">";
            int idx = checkedMapLookup(affectedColsAndRows.colNameToIndex, name, pbID);
            affectedColsAndRows.colsToDelete.push_back(idx);
        }

        // ==== DELETE AreaHydroLevel constraint ====
        {
            std::string name = "AreaHydroLevel::area<" + area + ">::hour<" + hourStr + ">";
            int idx = checkedMapLookup(affectedColsAndRows.rowNameToIndex, name, pbID);
            affectedColsAndRows.rowsToDelete.push_back(idx);
        }

        // ==== RESET HydroProd bounds ====
        {
            std::string name = "HydProd::area<" + area + ">::hour<" + hourStr + ">";
            int idx = checkedMapLookup(affectedColsAndRows.colNameToIndex, name, pbID);
            affectedColsAndRows.hydroProdCols.push_back(idx);
            affectedColsAndRows.hydroProdBounds.push_back(maxGen);
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
