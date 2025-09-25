
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

/// @brief Constructor
/// @param options The options for the problem generation
/// @param problems The problems to be modified
/// @param gridDefinition The grid definition
ProblemGenerationForWaterValueCalculation::ProblemGenerationForWaterValueCalculation(
  ConfigurationManager::ConfigDirectories directories,
  const ReservoirManagement& reservoirManagement,
  std::string solverName,
  unsigned int startWeek,
  unsigned int endWeek):
    directories(directories),
    reservoirManagement(reservoirManagement),
    startWeek(startWeek),
    endWeek(endWeek)
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
/// @return The path to the output mps file and strat and end weeks
UpdateProblemsResult ProblemGenerationForWaterValueCalculation::updateProblems(
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

    return {outputMpsPath, startWeek, endWeek};
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
                      if (startWeek <= pbId.week && pbId.week <= endWeek)
                      {
                          // needed if gridCollection contains multiple gridDefinitions
                          auto problem = std::make_shared<Problem>(pb.second->clone());
                          std::string pbName = "problem-" + std::to_string(pbId.year) + "-"
                                               + std::to_string(pbId.week) + "--optim-nb-1";
                          cleanProblemForBellmanCalculations(problem, pbName, gridDefinition, pbId);

                          problem->write_prob_mps(outputMpsPath / (pbName + ".mps"));
                      }
                  });

    return outputMpsPath;
}

/// @brief Clean the problem for the Bellman Values calculations
/// @param problem The problem to clean
/// @param gridDefinition The grid definition
/// @param week The week to clean
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
                  {gridDefinition.reservoirs.at(gridElement.area).max_generating[pbID.week - 1]
                   / Reservoir::hours_in_week});
            }
            addReservoirConstraints(problem, pbID);
        }
    }
}

void ProblemGenerationForWaterValueCalculation::addReservoirConstraints(
  std::shared_ptr<Problem> problem,
  Antares::Solver::WeeklyProblemId pbId)
{
    // ===== 1. Add variables =====
    std::vector<std::string> var_names;
    std::vector<double> bdl;
    std::vector<double> bdu;

    // x_s
    var_names.push_back("x_s");
    bdl.push_back(0.0);
    bdu.push_back(reservoirManagement.reservoir.capacity);

    // x_s_1
    var_names.push_back("x_s_1");
    bdl.push_back(0.0);
    bdu.push_back(reservoirManagement.reservoir.capacity);

    // U
    var_names.push_back("u");
    double lbU = -reservoirManagement.reservoir.max_pumping[pbId.week - 1]
                 * reservoirManagement.reservoir.efficiency;
    double ubU = reservoirManagement.reservoir.max_generating[pbId.week - 1];
    bdl.push_back(lbU);
    bdu.push_back(ubU);

    // y
    var_names.push_back("y");
    bdl.push_back(0.0);
    bdu.push_back(INFINITY);

    int nColsBeforeAdd = problem->get_ncols();
    // Add all variables at once
    {
        int nbVarToAdd = var_names.size();
        std::vector<int> mstart(nbVarToAdd, 0);
        std::vector<double> objs(nbVarToAdd, 0);
        std::vector<char> types(nbVarToAdd, 'C');

        solver_addcols(*problem, objs, mstart, {}, {}, bdl, bdu, types, var_names);
    }

    // // Column indices
    int col_x_s = nColsBeforeAdd;
    int col_x_s_1 = nColsBeforeAdd + 1;
    int col_U = nColsBeforeAdd + 2;
    int col_y = nColsBeforeAdd + 3;

    // ===== 2. Reservoir conservation constraint =====
    // x_s_1 - x_s + U <=/== inflow
    {
        std::vector<int> mclind = {col_x_s_1, col_x_s, col_U};
        std::vector<double> coeffs = {1.0, -1.0, 1.0};

        double inflow = reservoirManagement.reservoir.inflow[pbId.week - 1][pbId.year - 1];
        char qrtype = reservoirManagement.overflow ? 'L' : 'E';

        std::string cname = "ReservoirConservation::area<" + reservoirManagement.reservoir.area
                            + ">::week<" + std::to_string(pbId.week) + ">";

        solver_addrows(*problem, {qrtype}, {inflow}, {}, {0, 3}, mclind, coeffs, {cname});
    }

    // ===== 3. Penalty constraints =====
    if (pbId.week != endWeek - 1 || !reservoirManagement.final_level)
    {
        // y >= -penalty_bottom * (x_s_1 - bottom_rule_curve)
        {
            double rhs = reservoirManagement.penalty_bottom_rule_curve
                         * reservoirManagement.reservoir.bottom_rule_curve[pbId.week - 1];
            std::vector<int> mclind = {col_y, col_x_s_1};
            std::vector<double> coeffs = {1.0, reservoirManagement.penalty_bottom_rule_curve};
            char qrtype = 'G';
            std::string cname = "PenaltyForViolatingBottomRuleCurve::area<"
                                + reservoirManagement.reservoir.area + ">::week<"
                                + std::to_string(pbId.week) + ">";

            solver_addrows(*problem, {qrtype}, {rhs}, {}, {0, 2}, mclind, coeffs, {cname});
        }

        // y >= penalty_upper * (x_s_1 - upper_rule_curve)
        {
            double rhs = -reservoirManagement.penalty_upper_rule_curve
                         * reservoirManagement.reservoir.upper_rule_curve[pbId.week - 1];
            std::vector<int> mclind = {col_y, col_x_s_1};
            std::vector<double> coeffs = {1.0, -reservoirManagement.penalty_upper_rule_curve};
            char qrtype = 'G';
            std::string cname = "PenaltyForViolatingUpperRuleCurve::area<"
                                + reservoirManagement.reservoir.area + ">::week<"
                                + std::to_string(pbId.week) + ">";

            solver_addrows(*problem, {qrtype}, {rhs}, {}, {0, 2}, mclind, coeffs, {cname});
        }
    }
    else
    {
        // Final level case
        // y >= penalty_final_level * (x_s_1 - final_level)
        {
            double rhs = reservoirManagement.penalty_final_level * reservoirManagement.final_level;
            std::vector<int> mclind = {col_y, col_x_s_1};
            std::vector<double> coeffs = {1.0, reservoirManagement.penalty_final_level};
            char qrtype = 'G';
            std::string cname = "PenaltyForViolatingBottomRuleCurve::area<"
                                + reservoirManagement.reservoir.area + ">::week<"
                                + std::to_string(pbId.week) + ">";

            solver_addrows(*problem, {qrtype}, {rhs}, {}, {0, 2}, mclind, coeffs, {cname});
        }

        // y >= penalty_final_level * (final_level - x_s_1)
        {
            double rhs = -reservoirManagement.penalty_final_level * reservoirManagement.final_level;
            std::vector<int> mclind = {col_y, col_x_s_1};
            std::vector<double> coeffs = {1.0, -reservoirManagement.penalty_final_level};
            char qrtype = 'G';
            std::string cname = "PenaltyForViolatingUpperRuleCurve::area<"
                                + reservoirManagement.reservoir.area + ">::week<"
                                + std::to_string(pbId.week) + ">";

            solver_addrows(*problem, {qrtype}, {rhs}, {}, {0, 2}, mclind, coeffs, {cname});
        }
    }
}
