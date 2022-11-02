//
// Created by marechaljas on 08/11/22.
//

#pragma once

#include "IProblemVariablesProviderPort.h"
#include "LinkProblemsGenerator.h"

class ProblemVariablesZipAdapter : public IProblemVariablesProviderPort {
 public:
  ProblemVariablesZipAdapter(
      std::shared_ptr<ArchiveReader> reader, ProblemData data,
      const std::vector<ActiveLink>& vector,
      std::shared_ptr<ProblemGenerationLog::ProblemGenerationLogger> ptr);

  ProblemVariables Provide() override;
  std::shared_ptr<ArchiveReader> archive_reader_;
  const ProblemData problem_data_;
  const std::vector<ActiveLink>& links_;
  const std::shared_ptr<ProblemGenerationLog::ProblemGenerationLogger> logger_;
  void extract_variables(
      std::istringstream& variableFileContent,
      std::vector<std::string>& var_names,
      std::map<colId, ColumnsToChange>& p_ntc_columns,
      std::map<colId, ColumnsToChange>& p_direct_cost_columns,
      std::map<colId, ColumnsToChange>& p_indirect_cost_columns) const;
};
