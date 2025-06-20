
#include "antares-xpansion/lpnamer/main/ProblemGenerationForWaterValueCalculation.h"

#include <execution>
#include <iostream>
#include <utility>

#include <antares/api/solver.h>

#include "antares-xpansion/lpnamer/problem_modifier/XpansionProblemsFromAntaresProvider.h"
#include "malloc.h"

static const std::string LP_DIRNAME = "lp";

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

ProblemGenerationForWaterValueCalculation::ProblemGenerationForWaterValueCalculation(
  ProblemGenerationOptions& options):
    options_(options),
    configuration_manager_{options}
{
    mode_ = configuration_manager_.Mode();
}

void ProblemGenerationForWaterValueCalculation::setGridDefinition(
  std::shared_ptr<GridDefinition> gridDefinition)
{
    this->gridDefinition = gridDefinition;
}

namespace
{
bool islower(std::string_view str)
{
    return std::ranges::all_of(str, [](char c) { return std::islower(c); });
}
} // namespace

static std::string solverXpansionToSimulator(const SolverConfig& in)
{
    // in could be Cbc or CBC depending on whether it is defined or not in the
    // settings file
    // Use lowerCase in any case to be robust to these subtleties
    assert(islower(in.Name()));
    if (in.Name() == "xpress")
    {
        return "xpress";
    }
    if (in.Name() == "cbc" || in.Name() == "coin")
    {
        return "coin";
    }
    throw std::invalid_argument("Invalid solver");
}

void ProblemGenerationForWaterValueCalculation::performAntaresSimulation(
  const std::filesystem::path& output)
{
    Antares::Solver::Optimization::OptimizationOptions optOptions;

    optOptions.linearSolver = solverXpansionToSimulator(solver_config_);
    auto results = Antares::API::PerformSimulation(options_.StudyPath(), output, optOptions);

    /**
     * Antares simulator allocate a lot of memory
     * Even if there is no memory leak not all freed memory become available.
     * Allocator or OS may cache some memory to reuse it
     * With malloc_trim(0) we free all memory that is not used anymore to be reclaimed by the
     *program It is nescasssry to avoid allocating Xpansion memory on top of the unavailable memory
     *from simulator
     **/
#ifndef _WIN32
    malloc_trim(0);
#endif

    // Handle errors
    if (results.error)
    {
        throw LogUtils::XpansionError<std::runtime_error>("Antares simulation failed:\n\t"
                                                            + results.error->reason,
                                                          LOGLOCATION);
    }

    lps_ = std::move(results.antares_problems);
}

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

    // set_solver(directories_.study_dir, logger.get());

    if (mode_ == SimulationInputMode::ANTARES_API)
    {
        performAntaresSimulation(directories_.simulation_dir);
    }

    CleanProblemsForBellmanCalculations(directories_.xpansion_output_dir, log_file_path);

    return directories_.xpansion_output_dir;
}

void ProblemGenerationForWaterValueCalculation::CleanProblemsForBellmanCalculations(
  const std::filesystem::path& xpansion_output_dir,
  const std::filesystem::path& log_file_path)
{
    auto solver_log_manager = SolverLogManager(log_file_path);

    // vector of pair for parallelization
    // ref to WeeklyDataFromAntares to avoid copies
    std::vector<
      std::pair<Antares::Solver::WeeklyProblemId, Antares::Solver::WeeklyDataFromAntares&>>
      weekly_data;
    std::ranges::for_each(lps_.weeklyProblems,
                          [&weekly_data, this](auto& pair)
                          { weekly_data.emplace_back(pair.first, pair.second); });
    // Create directory for Bellman problems
    std::filesystem::create_directory(xpansion_output_dir / "mps");
    std::for_each(std::execution::seq,
                  weekly_data.begin(),
                  weekly_data.end(),
                  [&](const auto& weeklyDataByYearWeek)
                  {
                      auto&& [year_week, data] = weeklyDataByYearWeek;
                      XpansionProblemsFromAntaresProvider adapter(lps_);
                      auto problem = adapter.provideProblem("CBC", solver_log_manager, year_week);
                      cleanProblemForBellmanCalculations(problem, gridDefinition, year_week.week);
                      problem->write_prob_mps(xpansion_output_dir / "mps"
                                              / ("problem-" + std::to_string(year_week.year) + "-"
                                                 + std::to_string(year_week.week)
                                                 + "--optim-nb-1.mps"));
                  });
}

void cleanProblemForBellmanCalculations(std::shared_ptr<Problem> problem,
                                        std::shared_ptr<GridDefinition> gridDefinition,
                                        int week)
{
    for (const auto& gridElement: gridDefinition->gridElements)
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
