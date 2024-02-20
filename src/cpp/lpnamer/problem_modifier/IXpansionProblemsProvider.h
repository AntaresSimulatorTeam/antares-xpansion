//
// Created by marechaljas on 25/11/22.
//

#pragma once

#include "../model/Problem.h"

class IXpansionProblemsProvider {
 public:
  virtual ~IXpansionProblemsProvider() = default;
  [[nodiscard]] virtual std::vector<std::shared_ptr<Problem>> provideProblems(
      const std::string& solver_name,
      SolverLogManager& solver_log_manager) const = 0;
};
