//
// Created by marechaljas on 20/09/22.
//
#include "ACL/Stub/StudyInMemoryAdapter.h"
#include "AntaresStudyInMemoryAdapter.h"
#include "CoreHexagone/IForProvidingAntaresStudyPort.h"
#include "CoreHexagone/Model/Antares/Study.h"
#include "CoreHexagone/Model/Optimization/OptimizationProblem.h"
#include "CoreHexagone/Model/Solution/Solution.h"
#include "CoreHexagone/Model/Xpansion/Study.h"
#include "CoreHexagone/OptimizationProblemAdapter.h"
#include "CoreHexagone/ProblemSolver.h"
#include "gtest/gtest.h"

TEST(UseCase, GenerateOptimizationProblem) {
  // Given an Antares Study (<- Break it down ? in 1) The study, 2) The initial
  // optimization problem from the study)
  //  AND Given a Xpansion Study
  // WHEN I start the application
  // THEN Provide a optimized solution

  AntaresStudyInMemoryAdapter antares_study_adapter;
  AntaresStudy::Study antares_study = antares_study_adapter.Study("Study path");
  StudyInMemoryAdapter xpansion_study_adapter;
  XpansionStudy::Study xpansion_study =
      xpansion_study_adapter.Study("Study path");

  // Concrete implementation probably tested by itself with
  // InMemoryStudyAdapters
  OptimizationProblemAdapter
      problem_adapter;  // <- Probably need to replace with InMemory
  OptimizationProblem problem =
      problem_adapter.generateProblem(antares_study, xpansion_study);

  ProblemSolver solver;
  Solution solution = solver.Solve(problem);

  EXPECT_TRUE(true);  //<- Of course test the expected solution
}