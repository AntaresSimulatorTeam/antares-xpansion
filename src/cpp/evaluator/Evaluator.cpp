
#include "antares-xpansion/evaluator/Evaluator.h"

#include <fmt/core.h>
#include <regex>
#include <sstream>
#include <tbb/global_control.h>
#include <tbb/parallel_for_each.h>
#include <utility>

#include "antares-xpansion/benders/benders_core/BendersProblemFromFile.h"
#include "antares-xpansion/helpers/Timer.h"

using namespace PlainData;

/// @brief Constructor
/// @param logger Logger
/// @param problems map of subproblems to evaluate
/// @param solverName Name of the solver to use
/// @param nbThreads Number of threads to use
Evaluator::Evaluator(Logger logger,
                     std::map<Antares::Solver::WeeklyProblemId, std::shared_ptr<Problem>> problems,
                     std::filesystem::path studyDir,
                     std::string solverName,
                     int nbThreads):
    logger{std::move(logger)},
    problems(problems),
    solverName(solverName),
    studyDir(studyDir),
    nbThreads(nbThreads)
{
}

/// @brief Set the constraints RHS values for a given subproblem
/// @param rhsValues The RHS values to set
/// @param subProblem The subproblem
void Evaluator::SetConstraintsRHSValues(const std::map<std::string, double>& rhsValues,
                                        std::shared_ptr<Problem> subProblem)
{
    for (const auto& [constraintName, value]: rhsValues)
    {
        subProblem->fix_rhs_to(constraintName, value);
    }
}

/// @brief Get the name of the constraint in the mps file
/// @param id ID of the subproblem
/// @param area The name of the area
/// @param constraint The name of the constraint
/// @return The name of the constraint in the mps file
std::string Evaluator::GetConstraintName(const Antares::Solver::WeeklyProblemId id,
                                         const std::string& area,
                                         const std::string& constraint) const
{
    return fmt::format("{}::area<{}>::week<{}>", constraint, area, id.week - 1);
}

/// @brief Runs the ProcessSubproblem method in parallel for each subproblem
void Evaluator::Run()
{
    // Limiter TBB au nombre de cœurs physiques
    tbb::global_control limit(tbb::global_control::max_allowed_parallelism, nbThreads);

    tbb::parallel_for_each(problems.begin(),
                           problems.end(),
                           [this](auto& kv)
                           {
                               auto& [yearWeekId, subPb] = kv;
                               logger->display_message((std::stringstream()
                                                        << "Processing subproblem : year "
                                                        << yearWeekId.year << " week "
                                                        << yearWeekId.week)
                                                         .str(),
                                                       LogUtils::LOGLEVEL::INFO,
                                                       EVALUATOR_LOGGER_CONTEXT);
                               ProcessSubproblem(yearWeekId, subPb);
                           });
}

/// @brief Solve the subproblem and return the cost
/// @param problem The subproblem to solve
/// @return The data of the solved subproblem : cost and dualValues
SubProblemData Evaluator::SolveSubproblem(std::shared_ptr<Problem> problem)
{
    SubProblemData subPbData;
    Timer subproblem_timer;
    int status = problem->solve_lp();
    if (status != 0)
    {
        logger->display_message("ERROR: status for this problem was not 0. MPS file saved to disk "
                                "in output folder for analysis.",
                                LogUtils::LOGLEVEL::ERR,
                                EVALUATOR_LOGGER_CONTEXT);
        std::filesystem::path problemFileName("illformed_problem_year_"
                                              + std::to_string(problem->mc_year) + "_week_"
                                              + std::to_string(problem->week) + ".mps");
        problem->write_prob_mps(studyDir / problemFileName);
        logger->display_message("File saved at: " + studyDir.string(),
                                LogUtils::LOGLEVEL::ERR,
                                EVALUATOR_LOGGER_CONTEXT);
    }
    subPbData.subproblem_cost = problem->get_lp_value();
    subPbData.subproblem_timer = subproblem_timer.elapsed();
    int nbSimplexIter = problem->get_splex_num_of_ite_last();

    totalSimplexIter += nbSimplexIter;
    totalSubPbTimer += subPbData.subproblem_timer;

    return subPbData;
}
