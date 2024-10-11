//
// Created by marechaljas on 09/11/22.
//

#include "ProblemVariablesFileAdapter.h"

#include <utility>

#include "antares-xpansion/lpnamer/input_reader/VariableFileReader.h"

void ProblemVariablesFileAdapter::extract_variables(
    const std::filesystem::path& file, std::vector<std::string>& var_names,
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

  VariableFileReader variableReader(file, active_links_, variable_name_config,
                                    logger_);
  var_names = variableReader.getVariables();
  p_ntc_columns = variableReader.getNtcVarColumns();
  p_direct_cost_columns = variableReader.getDirectCostVarColumns();
  p_indirect_cost_columns = variableReader.getIndirectCostVarColumns();
}

ProblemVariables ProblemVariablesFileAdapter::Provide() {
  ProblemVariables result;
  extract_variables(lpdir_.parent_path() / problem_data_._variables_txt,
                    result.variable_names, result.ntc_columns,
                    result.direct_cost_columns, result.indirect_cost_columns);
  return result;
}

ProblemVariablesFileAdapter::ProblemVariablesFileAdapter(
    ProblemData data, std::vector<ActiveLink> vector_1,
    std::shared_ptr<ProblemGenerationLog::ProblemGenerationLogger> shared_ptr_1,
    std::filesystem::path path)
    : problem_data_(std::move(data)),
      active_links_(std::move(vector_1)),
      logger_(std::move(shared_ptr_1)),
      lpdir_(std::move(path)) {}
