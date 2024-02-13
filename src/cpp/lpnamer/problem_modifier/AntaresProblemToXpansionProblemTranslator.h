//
// Created by marechaljas on 22/11/22.
//

#pragma once

#include <antares/solver/simulation/LpsFromAntares.h>

#include "../model/Problem.h"

class AntaresProblemToXpansionProblemTranslator {
 public:
  [[nodiscard]] static std::shared_ptr<Problem> translateToXpansionProblem(
      const LpsFromAntares& lps, unsigned int year, unsigned int week,
      const std::string& solver_name, SolverLogManager& solver_log_manager);
  static std::vector<char> convertSignToLEG(char* data);
};
