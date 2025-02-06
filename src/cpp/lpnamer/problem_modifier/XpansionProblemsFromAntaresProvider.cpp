//
// Created by marechaljas on 18/11/22.
//

#include "antares-xpansion/lpnamer/problem_modifier/XpansionProblemsFromAntaresProvider.h"

#include <execution>
#include <memory>
#include <mutex>
#include <utility>

#include "antares-xpansion/lpnamer/model/Problem.h"
#include "antares-xpansion/lpnamer/problem_modifier/AntaresProblemToXpansionProblemTranslator.h"

XpansionProblemsFromAntaresProvider::XpansionProblemsFromAntaresProvider(
  const Antares::Solver::LpsFromAntares& lps):
    antares_hebdo_problems{lps}
{
}

std::vector<std::shared_ptr<Problem>> XpansionProblemsFromAntaresProvider::provideProblems(
  const std::string& solver_name,
  SolverLogManager& solver_log_manager) const
{
    std::vector<std::shared_ptr<Problem>> xpansion_problems;
    xpansion_problems.reserve(antares_hebdo_problems.weekCount());
    std::vector<std::pair<Antares::Solver::WeeklyProblemId, Antares::Solver::WeeklyDataFromAntares>> weekly_problems_vector;
    weekly_problems_vector.reserve(antares_hebdo_problems.weeklyProblems.size());
    for (const auto& [problem_id, hebdo_data] : antares_hebdo_problems.weeklyProblems) {
        weekly_problems_vector.emplace_back(problem_id, hebdo_data);
    }
    std::mutex mutex;
    std::for_each(
      std::execution::par,
      weekly_problems_vector.begin(),
      weekly_problems_vector.end(),
      [&](const auto& pair) {
          const auto& [problem_id, hebdo_data] = pair;
          auto problem = AntaresProblemToXpansionProblemTranslator::translateToXpansionProblem(
            antares_hebdo_problems,
            problem_id.year,
            problem_id.week,
            solver_name,
            solver_log_manager);
          std::lock_guard guard(mutex);
          xpansion_problems.emplace_back(problem);
      });
    return xpansion_problems;
}
