//
// Created by marechaljas on 22/11/22.
//

#pragma once

#include <cstdio>

#include "../model/Problem.h"
#include "LpsFromAntares.h"

class AntaresProblemToXpansionProblemTranslator {
 public:
  [[nodiscard]] static std::shared_ptr<Problem> translateToXpansionProblem(
      const LpsFromAntares& lps, unsigned int year, unsigned int week,
      const std::string& solver_name, FILE* log_file_ptr);
  static std::vector<char> convertSignToLEG(char* data);
};
