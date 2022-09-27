//
// Created by marechaljas on 27/09/22.
//

#pragma once

#include "Model/Antares/Study.h"
#include "Model/Optimization/OptimizationProblem.h"
#include "Model/Xpansion/Study.h"

/**
 * TODO extract interface
 */
class OptimizationProblemAdapter {
 public:
  [[nodiscard]] OptimizationProblem generateProblem(
      AntaresStudy::Study study, XpansionStudy::Study study_1) const;
};
