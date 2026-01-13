
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

/// @brief Launch the simulation and save the problems satisfying startWeek <= week <= endweek
ProblemGenerationForWaterValueCalculation::ProblemGenerationForWaterValueCalculation(
  ConfigurationManager::ConfigDirectories directories,
  const ReservoirManagement& reservoirManagement,
  Logger logger,
  const std::string& solverName,
  unsigned int startWeek,
  unsigned int endWeek,
  bool writePbFiles,
  const std::string& problemFormat):
    directories(directories),
    reservoirManagement(reservoirManagement),
    logger(std::move(logger)),
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
    auto solver_log_manager = SolverLogManager(directories.simulation_dir / "solver.log");
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
ProblemGenerationForWaterValueCalculation::updateProblems(const GridDefinition& gridDefinition)
{
    using namespace std::string_literals;

    const auto log_file_path = directories.simulation_dir / "lp"s / "ProblemGenerationLog.txt"s;

    CreateDirectories(directories.simulation_dir);
    auto logger = ProblemGenerationLog::BuildLogger(log_file_path,
                                                    std::cout,
                                                    "Problem Modification"s);

    auto modifiedProblems = CleanProblemsForBellmanCalculations(directories.simulation_dir,
                                                                log_file_path,
                                                                gridDefinition);

    return modifiedProblems;
}

/// @brief Clean the problems for the Bellman Values calculations
/// @param xpansion_output_dir The output directory
/// @param log_file_path The path to the log file
/// @param gridDefinition The gridDefinition
/// @return The modified problems
std::map<Antares::Solver::WeeklyProblemId, std::shared_ptr<Problem>>
ProblemGenerationForWaterValueCalculation::CleanProblemsForBellmanCalculations(
  const std::filesystem::path& xpansion_output_dir,
  const std::filesystem::path& log_file_path,
  const GridDefinition& gridDefinition)
{
    auto solver_log_manager = SolverLogManager(log_file_path);
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
              // needed if gridCollection contains multiple gridDefinitions
              std::shared_ptr<Problem> problem = std::make_shared<Problem>(*(pb.second->clone()));
              std::string pbName = "problem-" + std::to_string(pbId.year) + "-"
                                   + std::to_string(pbId.week) + "--optim-nb-1";
              logger->display_message("Modifying problem: " + pbName);
              cleanProblemForBellmanCalculations(problem, pbName, gridDefinition, pbId);
              logger->display_message("Problem: " + pbName + " modified");
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
  std::string& pbName,
  const GridDefinition& gridDefinition,
  Antares::Solver::WeeklyProblemId pbID)
{
    // Build index maps once
    int ncols = problem->get_ncols();
    int nrows = problem->get_nrows();

    std::vector<std::string> colNames = problem->get_col_names(0, ncols - 1);
    std::vector<std::string> rowNames = problem->get_row_names(0, nrows - 1);

    std::unordered_map<std::string, int> colNameToIndex;
    std::unordered_map<std::string, int> rowNameToIndex;

    for (int i = 0; i < ncols; ++i)
    {
        colNameToIndex[trimTrailingSpaces(colNames[i])] = i;
    }

    for (int i = 0; i < nrows; ++i)
    {
        rowNameToIndex[trimTrailingSpaces(rowNames[i])] = i;
    }

    // Collect indices to delete and bounds to change
    std::vector<int> colsToDelete;
    std::vector<int> rowsToDelete;
    std::vector<int> hydroProdCols;
    std::vector<double> hydroProdBounds;

    int weekStart = (pbID.week - 1) * 168;
    int weekEnd = pbID.week * 168;

    for (const auto& gridElement: gridDefinition.gridElements)
    {
        if (gridElement.problemName == "all" || gridElement.problemName == pbName)
        {
            double maxGen = gridDefinition.reservoirs.at(gridElement.area)
                              .max_generating[pbID.week - 1]
                            / Reservoir::hours_in_week;

            for (int hour = weekStart; hour < weekEnd; ++hour)
            {
                std::string hourStr = std::to_string(hour);
                const std::string& area = gridElement.area;

                // ==== DELETE HydroLevel ====
                {
                    std::string name = "HydroLevel::area<" + area + ">::hour<" + hourStr + ">";
                    int idx = checkedMapLookup(colNameToIndex, name, pbID);
                    colsToDelete.push_back(idx);
                }

                // ==== DELETE Overflow ====
                {
                    std::string name = "Overflow::area<" + area + ">::hour<" + hourStr + ">";
                    int idx = checkedMapLookup(colNameToIndex, name, pbID);
                    colsToDelete.push_back(idx);
                }

                // ==== DELETE AreaHydroLevel constraint ====
                {
                    std::string name = "AreaHydroLevel::area<" + area + ">::hour<" + hourStr + ">";
                    int idx = checkedMapLookup(rowNameToIndex, name, pbID);
                    rowsToDelete.push_back(idx);
                }

                // ==== RESET HydroProd bounds ====
                {
                    std::string name = "HydProd::area<" + area + ">::hour<" + hourStr + ">";
                    int idx = checkedMapLookup(colNameToIndex, name, pbID);
                    hydroProdCols.push_back(idx);
                    hydroProdBounds.push_back(maxGen);
                }
            }
        }
    }

    // Sort in descending order to preserve indices during deletion
    std::sort(colsToDelete.rbegin(), colsToDelete.rend());
    std::sort(rowsToDelete.rbegin(), rowsToDelete.rend());

    // Batch change bounds (must be done before deletions to preserve indices)
    if (!hydroProdCols.empty())
    {
        problem->chg_bounds(hydroProdCols,
                            std::vector<char>(hydroProdCols.size(), 'U'),
                            hydroProdBounds);
    }

    for (int idx: colsToDelete)
    {
        problem->del_cols(idx, idx);
    }

    for (int idx: rowsToDelete)
    {
        problem->del_rows(idx, idx);
    }
}
