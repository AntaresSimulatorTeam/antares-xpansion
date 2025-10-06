//
// Created by marechaljas on 22/11/22.
//

#pragma once

#include <span>
#include <unordered_map>

#include <antares/solver/lps/LpsFromAntares.h>

#include "antares-xpansion/lpnamer/model/Problem.h"

namespace AntaresProblemToXpansionProblemTranslator
{
[[nodiscard]] std::shared_ptr<Problem> translateToXpansionProblem(
  const Antares::Solver::LpsFromAntares& lps,
  unsigned int year,
  unsigned int week,
  const std::string& solver_name,
  SolverLogManager& solver_log_manager);
std::vector<char> convertSignToLEG(std::span<const char> data);
} // namespace AntaresProblemToXpansionProblemTranslator
