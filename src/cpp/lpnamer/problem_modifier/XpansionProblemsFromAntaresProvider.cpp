//
// Created by marechaljas on 18/11/22.
//

#include "XpansionProblemsFromAntaresProvider.h"

#include <utility>

#include "../model/Problem.h"
#include "AntaresProblemToXpansionProblemTranslator.h"

XpansionProblemsFromAntaresProvider::XpansionProblemsFromAntaresProvider(
    LpsFromAntares lps)
    : antares_hebdo_problems(std::move(lps)) {}

std::vector<std::shared_ptr<Problem>>
XpansionProblemsFromAntaresProvider::provideProblems(
    const std::string& solver_name,
    const std::filesystem::path& log_file_path) const {
  std::vector<std::shared_ptr<Problem>> xpansion_problems;
  xpansion_problems.reserve(
      XpansionProblemsFromAntaresProvider::antares_hebdo_problems._hebdo
          .size());
  for (const auto& [problem_id, hebdo_data] :
       XpansionProblemsFromAntaresProvider::antares_hebdo_problems._hebdo) {
    xpansion_problems.push_back(
        AntaresProblemToXpansionProblemTranslator::translateToXpansionProblem(
            XpansionProblemsFromAntaresProvider::antares_hebdo_problems,
            problem_id.year, problem_id.week, solver_name, log_file_path));
  }
  return xpansion_problems;
}