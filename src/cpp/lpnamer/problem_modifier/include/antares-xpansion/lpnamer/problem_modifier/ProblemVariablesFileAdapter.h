//
// Created by marechaljas on 09/11/22.
//

#pragma once

#include "antares-xpansion/lpnamer/problem_modifier/IProblemVariablesProviderPort.h"
#include "antares-xpansion/lpnamer/problem_modifier/LinkProblemsGenerator.h"
class ProblemVariablesFileAdapter : public IProblemVariablesProviderPort {
 public:
  ProblemVariablesFileAdapter(
      ProblemData data, std::vector<ActiveLink> vector_1,
      std::shared_ptr<ProblemGenerationLog::ProblemGenerationLogger>
          shared_ptr_1,
      std::filesystem::path path);
  ProblemVariables Provide() override;
  const ProblemData problem_data_;
  const std::vector<ActiveLink> active_links_;
  std::shared_ptr<ProblemGenerationLog::ProblemGenerationLogger> logger_;
  void extract_variables(
      const std::filesystem::path& file, std::vector<std::string>& var_names,
      std::map<colId, ColumnsToChange>& p_ntc_columns,
      std::map<colId, ColumnsToChange>& p_direct_cost_columns,
      std::map<colId, ColumnsToChange>& p_indirect_cost_columns) const;
  const std::filesystem::path lpdir_;
};
