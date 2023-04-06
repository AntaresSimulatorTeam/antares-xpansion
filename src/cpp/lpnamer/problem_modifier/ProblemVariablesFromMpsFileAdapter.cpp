//
// Created by marechaljas on 09/11/22.
//

#include "ProblemVariablesFromMpsFileAdapter.h"

#include <utility>

#include "VariableFileReader.h"

struct VariablesFields {
  std::string name;
  int link_id;
  int time_step;
};

// struct NTCVariablesFields : public VariablesFields {
//   int pays;
// };
VariablesFields VariableFieldsFromVariableName(const std::string& var_name) {
  auto vect_fields = common_lpnamer::split(var_name, '_');

  if (vect_fields.size() == 3) {
    return {vect_fields[0], std::atoi(vect_fields[1].c_str()),
            std::atoi(vect_fields[2].c_str())};

  } else {
    return {vect_fields[0], std::atoi(vect_fields[2].c_str()),
            std::atoi(vect_fields[3].c_str())};
  }
}
// VariablesFields NTCVariablesFieldsFromVariableName(
//     const std::string& var_name) {
//   auto vect_fields = common_lpnamer::split(var_name, '_');
//   VariablesFields result;
//   result.name = vect_fields[0];
//   result.link_id = std::atoi(vect_fields[2].c_str());
//   result.time_step = std::atoi(vect_fields[3].c_str());
// };

// void ParseVarNames(
//     std::shared_ptr<Problem> problem,
//     const VariableFileReadNameConfiguration& variable_name_config,
//     const std::vector<std::string>& var_names,
//     std::map<colId, ColumnsToChange>& p_ntc_columns,
//     std::map<colId, ColumnsToChange>& p_direct_cost_columns,
//     std::map<colId, ColumnsToChange>& p_indirect_cost_columns) {
//   for (const auto& var_name : var_names) {
//     if (auto str = common_lpnamer::split(var_name, '_');
//         str[0] == variable_name_config.ntc_variable_name) {
//       p_ntc_columns[problem->get_col_index(var_name)] = {}
//     updateMapColumn(lin)
//     }
//   }
// }

void ProblemVariablesFromMpsFileAdapter::extract_variables(
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

  //   VariableFileReader variableReader(file, active_links_,
  //   variable_name_config,
  //                                     logger_);

  var_names = problem_->get_col_names(0, problem_->get_ncols() - 1);
  //   p_ntc_columns = variableReader.getNtcVarColumns();
  //   p_direct_cost_columns = variableReader.getDirectCostVarColumns();
  //   p_indirect_cost_columns = variableReader.getIndirectCostVarColumns();

  for (const auto& var_name : var_names) {
    auto var_fields = VariableFieldsFromVariableName(var_name);
    if (var_fields.name == variable_name_config.ntc_variable_name) {
      updateMapColumn(active_links_, var_fields.link_id,
                      problem_->get_col_index(var_name), var_fields.time_step,
                      p_ntc_columns);
    } else if (var_fields.name ==
               variable_name_config.cost_origin_variable_name) {
      updateMapColumn(active_links_, var_fields.link_id,
                      problem_->get_col_index(var_name), var_fields.time_step,
                      p_direct_cost_columns);
    } else if (var_fields.name ==
               variable_name_config.cost_extremite_variable_name) {
      updateMapColumn(active_links_, var_fields.link_id,
                      problem_->get_col_index(var_name), var_fields.time_step,
                      p_indirect_cost_columns);
    }
  }
}

ProblemVariables ProblemVariablesFromMpsFileAdapter::Provide() {
  ProblemVariables result;
  extract_variables(result.variable_names, result.ntc_columns,
                    result.direct_cost_columns, result.indirect_cost_columns);
  return result;
}

ProblemVariablesFromMpsFileAdapter::ProblemVariablesFromMpsFileAdapter(
    std::shared_ptr<Problem> problem, std::vector<ActiveLink> vector_1,
    std::shared_ptr<ProblemGenerationLog::ProblemGenerationLogger> shared_ptr_1)
    : problem_(std::move(problem)),
      active_links_(std::move(vector_1)),
      logger_(std::move(shared_ptr_1)) {}
