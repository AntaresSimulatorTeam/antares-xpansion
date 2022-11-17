//
// Created by marechaljas on 18/11/22.
//

#pragma once

#include "IXpansionProblemsProvider.h"
#include "LpsFromAntares.h"
#include "Problem.h"
class XpansionProblemsFromAntaresProvider : public IXpansionProblemsProvider {
 public:
  explicit XpansionProblemsFromAntaresProvider(LpsFromAntares antares);
  [[nodiscard]] std::vector<std::shared_ptr<Problem>> provideProblems(
      const std::string& solver_name) const override;
  LpsFromAntares antares_hebdo_problems;
};
