//
// Created by marechaljas on 08/11/22.
//

#include "ProblemVariablesZipAdapter.h"

#include <utility>

#include "ArchiveReader.h"
#include "LinkProblemsGenerator.h"
void ProblemVariablesZipAdapter::extract_variables(
    std::istringstream& variableFileContent,
    std::vector<std::string>& var_names,
    std::map<colId, ColumnsToChange>& p_ntc_columns,
    std::map<colId, ColumnsToChange>& p_direct_cost_columns,
    std::map<colId, ColumnsToChange>& p_indirect_cost_columns) const {
  // List of variables
  VariableFileReadNameConfiguration variable_name_config;
  variable_name_config.ntc_variable_name = "ValeurDeNTCOrigineVersExtremite";
  variable_name_config.cost_origin_variable_name =
      "CoutOrigineVersExtremiteDeLInterconnexion";
  variable_name_config.cost_extremite_variable_name =
      "CoutExtremiteVersOrigineDeLInterconnexion";

  auto variableReader = VariableFileReader(variableFileContent, links_,
                                           variable_name_config, logger_);
  var_names = variableReader.getVariables();
  p_ntc_columns = variableReader.getNtcVarColumns();
  p_direct_cost_columns = variableReader.getDirectCostVarColumns();
  p_indirect_cost_columns = variableReader.getIndirectCostVarColumns();
}

ProblemVariables ProblemVariablesZipAdapter::Provide() {
  auto variableFileContent =
      archive_reader_->ExtractFileInStringStream(problem_data_._variables_txt);

  ProblemVariables result;
  extract_variables(variableFileContent, result.variable_names,
                    result.ntc_columns, result.direct_cost_columns,
                    result.indirect_cost_columns);

  return result;
}
ProblemVariablesZipAdapter::ProblemVariablesZipAdapter(
    std::shared_ptr<ArchiveReader> reader, ProblemData data,
    const std::vector<ActiveLink>& vector,
    std::shared_ptr<ProblemGenerationLog::ProblemGenerationLogger> ptr)
    : archive_reader_(std::move(reader)),
      problem_data_(std::move(data)),
      links_(vector),
      logger_(std::move(ptr)) {}
