//
// Created by marechaljas on 09/11/22.
//

#include <utility>

#include "ProblemVariablesFromProblemAdapter.h"
// #include "VariableFileReader.h"

const std::string SEPARATOR = "::";
const std::string AREA_SEPARATOR = "$$";
const char WITHESPACESUBSTITUTE = '*';

struct VariableNameComposition {
  std::string name;
  std::string origin;
  std::string destination;
  int time_step;
};

std::string StringBetweenChevrons(const std::string& input) {
  // input = X<Y>
  return common_lpnamer::split(common_lpnamer::split(input, '<')[1], '>')[0];
}

void ReadLinkZones(const std::string& input, std::string& origin,
                   std::string& destination) {
  // input format should be link<area1$$area2>
  const auto zones =
      common_lpnamer::split(StringBetweenChevrons(input), AREA_SEPARATOR);
  origin = zones[0];
  std::replace(origin.begin(), origin.end(), WITHESPACESUBSTITUTE, ' ');
  destination = zones[1];
  std::replace(destination.begin(), destination.end(), WITHESPACESUBSTITUTE,
               ' ');
}

int ReadTimeStep(const std::string& input) {
  // input format should be x<timeStep>
  return std::atoi(StringBetweenChevrons(input).c_str());
}

void updateMapColumn(const std::vector<ActiveLink>& links,
                     const std::string& link_origin,
                     const std::string& link_destination, colId id,
                     int time_step,
                     std::map<linkId, ColumnsToChange>& mapColumn) {
  auto it =
      std::find_if(links.begin(), links.end(),
                   [&link_origin, &link_destination](const ActiveLink& link) {
                     return link.get_linkor() == link_origin &&
                            link.get_linkex() == link_destination;
                   });

  if (it != links.end()) {
    mapColumn[it->get_idLink()].push_back({id, time_step});
  }
}

// VariableNameComposition VariableFieldsFromVariableName(
//     const std::string& var_name) {
//   auto vect_fields = common_lpnamer::split(var_name, SEPARATOR);
//   VariableNameComposition result;
//   if (vect_fields.size() == 3) {
//     result.name = vect_fields[0];
//     ReadLinkZones(vect_fields[1], result.origin, result.destination);
//     result.time_step = ReadTimeStep(vect_fields[2]);
//   }

//   return result;
// }

void ProblemVariablesFromProblemAdapter::extract_variables(
    std::vector<std::string>& var_names,
    std::map<colId, ColumnsToChange>& p_ntc_columns,
    std::map<colId, ColumnsToChange>& p_direct_cost_columns,
    std::map<colId, ColumnsToChange>& p_indirect_cost_columns) const {
  // List of variables
  VariableFileReadNameConfiguration variable_name_config;
  variable_name_config.ntc_variable_name = "NTCDirect";
  variable_name_config.cost_origin_variable_name = "IntercoDirectCost";
  variable_name_config.cost_extremite_variable_name = "IntercoInDirectCost";

  var_names = problem_->get_col_names(0, problem_->get_ncols() - 1);
  std::string origin;
  std::string destination;
  for (const auto& var_name : var_names) {
    auto split_name = common_lpnamer::split(var_name, SEPARATOR);

    if (split_name[0] == variable_name_config.ntc_variable_name) {
      ReadLinkZones(split_name[1], origin, destination);

      updateMapColumn(active_links_, origin, destination,
                      problem_->get_col_index(var_name),
                      ReadTimeStep(split_name[2]), p_ntc_columns);
    } else if (split_name[0] ==
               variable_name_config.cost_origin_variable_name) {
      ReadLinkZones(split_name[1], origin, destination);
      updateMapColumn(active_links_, origin, destination,
                      problem_->get_col_index(var_name),
                      ReadTimeStep(split_name[2]), p_direct_cost_columns);
    } else if (split_name[0] ==
               variable_name_config.cost_extremite_variable_name) {
      ReadLinkZones(split_name[1], origin, destination);
      updateMapColumn(active_links_, origin, destination,
                      problem_->get_col_index(var_name),
                      ReadTimeStep(split_name[2]), p_indirect_cost_columns);
    }
  }
}

ProblemVariables ProblemVariablesFromProblemAdapter::Provide() {
  ProblemVariables result;
  extract_variables(result.variable_names, result.ntc_columns,
                    result.direct_cost_columns, result.indirect_cost_columns);
  return result;
}

ProblemVariablesFromProblemAdapter::ProblemVariablesFromProblemAdapter(
    std::shared_ptr<Problem> problem, std::vector<ActiveLink> vector_1,
    std::shared_ptr<ProblemGenerationLog::ProblemGenerationLogger> shared_ptr_1)
    : problem_(std::move(problem)),
      active_links_(std::move(vector_1)),
      logger_(std::move(shared_ptr_1)) {}
