//
// Created by marechaljas on 27/09/22.
//

#pragma once

#include "Model/Optimization/OptimizationProblem.h"
#include "Model/Solution/Solution.h"

/**
 * TODO extract interface
 */
class ProblemSolver {
 public:
  [[nodiscard]] Solution Solve(OptimizationProblem problem) const;
};
