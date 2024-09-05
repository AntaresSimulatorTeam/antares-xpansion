//
// Created by marechaljas on 22/11/22.
//

#pragma once

#include <antares/solver/lps/LpsFromAntares.h>

#include <span>

#include "../model/Problem.h"

class AntaresProblemToXpansionProblemTranslator {
 public:
  [[nodiscard]] static std::shared_ptr<Problem> translateToXpansionProblem(
      const Antares::Solver::LpsFromAntares& lps, unsigned int year, unsigned int week,
      const std::string& solver_name, SolverLogManager& solver_log_manager);
  static std::vector<char> convertSignToLEG(std::span<char> data);
  static void roundTo10Digit(Antares::Solver::ConstantDataFromAntares& constant,
                             Antares::Solver::WeeklyDataFromAntares& hebdo);
};
