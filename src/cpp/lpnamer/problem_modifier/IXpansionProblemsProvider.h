//
// Created by marechaljas on 25/11/22.
//

#pragma once

#include <cstdio>
#include <filesystem>

#include "../model/Problem.h"
#include "LpsFromAntares.h"
class IXpansionProblemsProvider {
 public:
  virtual ~IXpansionProblemsProvider() {
    if (log_file_ptr != NULL) {
      fclose(log_file_ptr);
      log_file_ptr = NULL;
    }
  }
  [[nodiscard]] virtual std::vector<std::shared_ptr<Problem>> provideProblems(
      const std::string& solver_name,
      const std::filesystem::path& log_file_path) = 0;

 public:
  FILE* log_file_ptr = NULL;
};
