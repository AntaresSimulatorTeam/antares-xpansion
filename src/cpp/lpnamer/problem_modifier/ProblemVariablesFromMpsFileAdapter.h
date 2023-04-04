#pragma once

#include "IProblemVariablesProviderPort.h"
#include "LinkProblemsGenerator.h"

class ProblemVariablesFromMpsFileAdapter
    : public IProblemVariablesProviderPort {
 public:
  ProblemVariablesFromMpsFileAdapter(
      std::shared_ptr<Problem> problem, std::vector<struct ActiveLink> vector_1,
      std::shared_ptr<ProblemGenerationLog::ProblemGenerationLogger>
          shared_ptr_1);
  ProblemVariables Provide() override;

 private:
  void extract_variables(
      std::vector<std::string>& var_names,
      std::map<colId, ColumnsToChange>& p_ntc_columns,
      std::map<colId, ColumnsToChange>& p_direct_cost_columns,
      std::map<colId, ColumnsToChange>& p_indirect_cost_columns) const;

  std::shared_ptr<Problem> problem_;
  const std::vector<struct ActiveLink> active_links_;
  std::shared_ptr<ProblemGenerationLog::ProblemGenerationLogger> logger_;
};
