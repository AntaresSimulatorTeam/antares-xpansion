//
// Created by marechaljas on 18/11/22.
//

#pragma once

#include "../model/Problem.h"
#include "IXpansionProblemsProvider.h"
#include "LpsFromAntares.h"

class XpansionProblemsFromAntaresProvider : public IXpansionProblemsProvider {
 public:
  explicit XpansionProblemsFromAntaresProvider(LpsFromAntares antares);
  [[nodiscard]] std::vector<std::shared_ptr<Problem>> provideProblems(
      const std::string& solver_name,
      SolverLogManager& solver_log_manager) const override;
  LpsFromAntares antares_hebdo_problems;
};
