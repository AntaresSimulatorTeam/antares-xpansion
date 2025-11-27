
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
              std::shared_ptr<Problem> problem(pb.second->clone());
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

template<typename Func>
int checked_index(Func getter,             // e.g. lambda calling get_col_index / get_row_index
                  const std::string& name, // variable or constraint name
                  const std::string& pbID  // problem ID or descriptor
)
{
    int idx = getter(name);
    if (idx == -1)
    {
        throw std::runtime_error("Index not found for '" + name + "' in problem " + pbID);
    }
    return idx;
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
  std::string& pbName,
  const GridDefinition& gridDefinition,
  Antares::Solver::WeeklyProblemId pbID)
{
    for (const auto& gridElement: gridDefinition.gridElements)
    {
        if (gridElement.problemName == "all" || gridElement.problemName == pbName)
        {
            for (int hour = (pbID.week - 1) * 168; hour < pbID.week * 168; ++hour)
            {
                // ==== DELETE HydroLevel ====
                {
                    std::string name = "HydroLevel::area<" + gridElement.area + ">::hour<"
                                       + std::to_string(hour) + ">";
                    int idx = checked<&Problem::get_col_index>(problem.get(), name, pbID);
                    problem->del_cols(idx, idx);
                }

                // ==== DELETE Overflow ====
                {
                    std::string name = "Overflow::area<" + gridElement.area + ">::hour<"
                                       + std::to_string(hour) + ">";
                    int idx = checked<&Problem::get_col_index>(problem.get(), name, pbID);
                    problem->del_cols(idx, idx);
                }

                // ==== DELETE AreaHydroLevel constraint ====
                {
                    std::string name = "AreaHydroLevel::area<" + gridElement.area + ">::hour<"
                                       + std::to_string(hour) + ">";
                    int idx = checked<&Problem::get_row_index>(problem.get(), name, pbID);
                    problem->del_rows(idx, idx);
                }

                // ==== RESET HydroProd bounds ====
                {
                    std::string name = "HydProd::area<" + gridElement.area + ">::hour<"
                                       + std::to_string(hour) + ">";
                    int idx = checked<&Problem::get_col_index>(problem.get(), name, pbID);

                    problem->chg_bounds(
                      {idx},
                      {'U'},
                      {gridDefinition.reservoirs.at(gridElement.area).max_generating[pbID.week - 1]
                       / Reservoir::hours_in_week});
                }
            }
        }
    }
}
