
#include "antares-xpansion/lpnamer/main/ProblemGenerationOptimSimu.h"

#include <tbb/parallel_for_each.h>
#include <utility>

#include <antares/api/singleProblemGetter.h>
#include <antares/api/solver.h>

#include "antares-xpansion/lpnamer/problem_modifier/XpansionProblemsFromAntaresProvider.h"
#include "malloc.h"

/// @brief Launch the simulation and save the problems satisfying startWeek <= week <= endweek
ProblemGenerationOptimSimu::ProblemGenerationOptimSimu(
  ConfigurationManager::ConfigDirectories directories,
  Logger logger,
  std::shared_ptr<ProblemManager> problemManager,
  unsigned int startWeek,
  unsigned int endWeek):
    directories(directories),
    logger(std::move(logger)),
    problemManager(problemManager),
    startWeek(startWeek),
    endWeek(endWeek)
{
    loadProblemsFromAntares();
}

void ProblemGenerationOptimSimu::loadProblemsFromAntares()
{
    Antares::Solver::SingleProblemGetter spg(directories.study_dir);
    if (spg.areWeeksIndependent())
    {
        logger->display_message("Weeks are independent, using optimized problem generation");
        generateAntaresProblems(spg);
    }
    else
    {
        logger->display_message("Weeks are dependent, performing full Antares simulation");
        performAntaresSimulation();
    }
}

void ProblemGenerationOptimSimu::generateAntaresProblems(Antares::Solver::SingleProblemGetter& spg)
{
    Antares::Solver::LpsFromAntares lps;
    lps.setConstantData(spg.getConstantData());

    spg.writeNTCTimeSeries(directories.simulation_dir);
    spg.writeStudyDescriptionFiles(directories.simulation_dir);

    for (const auto& problem_id: spg.getProblemIds())
    {
        Antares::Solver::WeeklyProblemId fixed{problem_id.year + 1, problem_id.week};
        lps.addWeeklyData(fixed, spg.getWeeklyData(problem_id));
    }

#ifndef _WIN32
    malloc_trim(0);
#endif

    lpsToProblems(lps);
}

void ProblemGenerationOptimSimu::performAntaresSimulation()
{
    Antares::Solver::Optimization::OptimizationOptions optOptions;
    optOptions.firstOptimOptions.solverName = problemManager->solverName();
    optOptions.secondOptimOptions.solverName = problemManager->solverName();

    if (problemManager->solverName() == SolverConfig("xpress"))
    {
        optOptions.firstOptimOptions.solverParameters = "PRESOLVE 1";
        optOptions.secondOptimOptions.solverParameters = "PRESOLVE 1";
    }

    auto [lps, error] = Antares::API::PerformSimulation(directories.study_dir,
                                                        directories.simulation_dir,
                                                        optOptions);

#ifndef _WIN32
    malloc_trim(0);
#endif

    if (error)
    {
        throw LogUtils::XpansionError<std::runtime_error>("Antares simulation failed:\n\t"
                                                            + error->reason,
                                                          LOGLOCATION);
    }

    lpsToProblems(lps);
}

void ProblemGenerationOptimSimu::lpsToProblems(const Antares::Solver::LpsFromAntares& lps)
{
    XpansionProblemsFromAntaresProvider adapter(lps);
    for (const auto& [pbId, _]: lps.weeklyProblems)
    {
        auto problem = adapter.provideProblem(problemManager->solverName() == SolverConfig("xpress")
                                                ? "xpress"
                                                : "CBC",
                                              problemManager->solverLogManager(),
                                              pbId);
        problemManager->setProblem(pbId, problem);
    }

    auto problems = problemManager->getProblems();
    if (!problems.empty())
    {
        startWeek = std::max(startWeek, problems.begin()->first.week);
        endWeek = std::min(endWeek, problems.rbegin()->first.week);
    }
}
