//
// Created by marechaljas on 18/11/22.
//

#include "XpansionProblemsFromAntaresProvider.h"

#include <utility>

#include "AntaresProblemToXpansionProblemTranslator.h"
#include "Problem.h"
XpansionProblemsFromAntaresProvider::XpansionProblemsFromAntaresProvider(
    LpsFromAntares lps)
    : antares_hebdo_problems(std::move(lps)) {}

std::vector<std::shared_ptr<Problem>>
XpansionProblemsFromAntaresProvider::provideProblems(
    const std::string& solver_name) const {
  std::vector<std::shared_ptr<Problem>> xpansion_problems;
  xpansion_problems.reserve(
      XpansionProblemsFromAntaresProvider::antares_hebdo_problems._hebdo
          .size());
  for (const auto& [problem_id, hebdo_data] :
       XpansionProblemsFromAntaresProvider::antares_hebdo_problems._hebdo) {
    xpansion_problems.push_back(
        AntaresProblemToXpansionProblemTranslator::translateToXpansionProblem(
            XpansionProblemsFromAntaresProvider::antares_hebdo_problems,
            problem_id.year, problem_id.week, solver_name));
  }
  return xpansion_problems;
}