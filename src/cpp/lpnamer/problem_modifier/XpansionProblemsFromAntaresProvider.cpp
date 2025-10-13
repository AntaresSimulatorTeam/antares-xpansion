#include "antares-xpansion/lpnamer/problem_modifier/XpansionProblemsFromAntaresProvider.h"

#include <execution>
#include <memory>
#include <mutex>
#include <ranges>
#include <utility>

#include "antares-xpansion/lpnamer/model/Problem.h"
#include "antares-xpansion/lpnamer/problem_modifier/AntaresProblemToXpansionProblemTranslator.h"

XpansionProblemsFromAntaresProvider::XpansionProblemsFromAntaresProvider(
  const Antares::Solver::LpsFromAntares& lps):
    antares_hebdo_problems{lps}
{
}

auto XpansionProblemsFromAntaresProvider::provideProblem(
  const std::string& solver_name,
  SolverLogManager& solver_log_manager,
  const Antares::Solver::WeeklyProblemId& problem_id) const -> std::shared_ptr<Problem>
{
    auto problem = AntaresProblemToXpansionProblemTranslator::translateToXpansionProblem(
      antares_hebdo_problems,
      problem_id.year,
      problem_id.week,
      solver_name,
      solver_log_manager,
      rename_utils_);
    return problem;
}

std::vector<std::shared_ptr<Problem>> XpansionProblemsFromAntaresProvider::provideProblems(
  const std::string& solver_name,
  SolverLogManager& solver_log_manager) const
{
    std::vector<std::shared_ptr<Problem>> xpansion_problems;
    xpansion_problems.reserve(antares_hebdo_problems.weekCount());
    std::vector<Antares::Solver::WeeklyProblemId> problem_ids;
    problem_ids.reserve(antares_hebdo_problems.weeklyProblems.size());
    for (const auto& problem_id: antares_hebdo_problems.weeklyProblems | std::views::keys)
    {
        problem_ids.emplace_back(problem_id);
    }
    std::mutex mutex;
    std::for_each(std::execution::par,
                  problem_ids.begin(),
                  problem_ids.end(),
                  [&](const auto& id)
                  {
                      auto problem = provideProblem(solver_name, solver_log_manager, id);
                      std::lock_guard guard(mutex);
                      xpansion_problems.push_back(problem);
                  });
    return xpansion_problems;
}
