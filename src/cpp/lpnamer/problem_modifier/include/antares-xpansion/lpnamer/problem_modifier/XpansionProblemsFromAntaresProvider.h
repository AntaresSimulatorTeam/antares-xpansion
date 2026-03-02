#pragma once

#include <antares/solver/lps/LpsFromAntares.h>

#include "IXpansionProblemsProvider.h"
#include "RenameUtils.h"
#include "antares-xpansion/lpnamer/helper/ProblemGenerationLogger.h"
#include "antares-xpansion/lpnamer/model/Problem.h"

class XpansionProblemsFromAntaresProvider: public IXpansionProblemsProvider
{
public:
    explicit XpansionProblemsFromAntaresProvider(
      const Antares::Solver::LpsFromAntares& lps,
      ProblemGenerationLog::ProblemGenerationLogger* logger = nullptr);
    std::shared_ptr<Problem> provideProblem(
      const std::string& solver_name,
      SolverLogManager& solver_log_manager,
      const Antares::Solver::WeeklyProblemId& problem_id) const;
    [[nodiscard]] std::vector<std::shared_ptr<Problem>> provideProblems(
      const std::string& solver_name,
      SolverLogManager& solver_log_manager) const override;
    const Antares::Solver::LpsFromAntares& antares_hebdo_problems;

private:
    ProblemGenerationLog::ProblemGenerationLogger* logger_;
    RenameUtils rename_utils_;
};
