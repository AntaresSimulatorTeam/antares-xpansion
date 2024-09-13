#pragma once

#include "IProblemVariablesProviderPort.h"
#include "LinkProblemsGenerator.h"

class ProblemVariablesFromProblemAdapter
    : public IProblemVariablesProviderPort {
 public:
  ProblemVariablesFromProblemAdapter(
      std::shared_ptr<Problem> problem, std::vector<ActiveLink> vector_1,
      std::shared_ptr<ProblemGenerationLog::ProblemGenerationLogger>
          shared_ptr_1);
  ProblemVariables Provide() override;

 private:
  void extract_variables(
      std::map<colId, ColumnsToChange>& p_ntc_columns,
      std::map<colId, ColumnsToChange>& p_direct_cost_columns,
      std::map<colId, ColumnsToChange>& p_indirect_cost_columns) const;

  std::shared_ptr<Problem> problem_;
  const std::vector<ActiveLink> active_links_;
  std::shared_ptr<ProblemGenerationLog::ProblemGenerationLogger> logger_;
};
