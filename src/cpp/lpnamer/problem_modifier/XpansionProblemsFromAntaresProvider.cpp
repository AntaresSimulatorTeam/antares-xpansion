//
// Created by marechaljas on 18/11/22.
//

#include "antares-xpansion/lpnamer/problem_modifier/XpansionProblemsFromAntaresProvider.h"

#include <utility>

#include "antares-xpansion/lpnamer/model/Problem.h"
#include "antares-xpansion/lpnamer/problem_modifier/AntaresProblemToXpansionProblemTranslator.h"

XpansionProblemsFromAntaresProvider::XpansionProblemsFromAntaresProvider(
    Antares::Solver::LpsFromAntares lps)
    : antares_hebdo_problems(std::move(lps)) {}

std::vector<std::shared_ptr<Problem>>
XpansionProblemsFromAntaresProvider::provideProblems(
    const std::string& solver_name,
    SolverLogManager& solver_log_manager) const
{
  std::vector<std::shared_ptr<Problem>> xpansion_problems;
  xpansion_problems.reserve(
      antares_hebdo_problems.weekCount());
  for (const auto& [problem_id, hebdo_data] :
       antares_hebdo_problems.weeklyProblems) {
    xpansion_problems.push_back(
        AntaresProblemToXpansionProblemTranslator::translateToXpansionProblem(
            antares_hebdo_problems,
            problem_id.year, problem_id.week, solver_name, solver_log_manager));
  }
  return xpansion_problems;
}