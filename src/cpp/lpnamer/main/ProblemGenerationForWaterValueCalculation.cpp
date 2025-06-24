
#include "antares-xpansion/lpnamer/main/ProblemGenerationForWaterValueCalculation.h"

#include <execution>
#include <iostream>
#include <utility>

#include <antares/api/solver.h>

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
  ProblemGenerationOptions& options,
  const std::map<Antares::Solver::WeeklyProblemId, std::shared_ptr<Problem>> problems,
  const GridDefinition& gridDefinition):
    options_(options),
    configuration_manager_(options),
    problems(problems),
    gridDefinition(gridDefinition)
{
}

/// @brief Update the problems for the water value calculation
/// @return The path to the output mps file
std::filesystem::path ProblemGenerationForWaterValueCalculation::updateProblems()
{
    using namespace std::string_literals;
    directories_ = configuration_manager_.Directories();

    const auto log_file_path = directories_.xpansion_output_dir / "lp"s
                               / "ProblemGenerationLog.txt"s;

    CreateDirectories(directories_.xpansion_output_dir);
    auto logger = ProblemGenerationLog::BuildLogger(log_file_path,
                                                    std::cout,
                                                    "Problem Generation"s);

    auto outputMpsPath = CleanProblemsForBellmanCalculations(directories_.xpansion_output_dir,
                                                             log_file_path);

    return outputMpsPath;
}

/// @brief Clean the problems for the Bellman Values calculations
/// @param xpansion_output_dir The output directory
/// @param log_file_path The path to the log file
/// @return The path to the output mps file
std::filesystem::path
ProblemGenerationForWaterValueCalculation::CleanProblemsForBellmanCalculations(
  const std::filesystem::path& xpansion_output_dir,
  const std::filesystem::path& log_file_path)
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

                      cleanProblemForBellmanCalculations(problem, gridDefinition, pbId.week);

                      problem->write_prob_mps(outputMpsPath
                                              / ("problem-" + std::to_string(pbId.year) + "-"
                                                 + std::to_string(pbId.week) + "--optim-nb-1.mps"));
                  });

    return outputMpsPath;
}

/// @brief Clean the problem for the Bellman Values calculations
/// @param problem The problem to clean
/// @param gridDefinition The grid definition
/// @param week The week to clean
void cleanProblemForBellmanCalculations(std::shared_ptr<Problem> problem,
                                        const GridDefinition& gridDefinition,
                                        int week)
{
    for (const auto& gridElement: gridDefinition.gridElements)
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
        }
    }
}
