//
// Created by marechaljas on 09/11/22.
//

#include "antares-xpansion/lpnamer/problem_modifier/ProblemVariablesFromProblemAdapter.h"

#include <utility>

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
  return StringManip::split(StringManip::split(input, '<')[1], '>')[0];
}

void ReadLinkZones(const std::string& input, std::string& origin,
                   std::string& destination) {
  // input format should be link<area1$$area2>
  const auto zones =
      StringManip::split(StringBetweenChevrons(input), AREA_SEPARATOR);
  origin = zones[0];
  std::replace(origin.begin(), origin.end(), WITHESPACESUBSTITUTE, ' ');
  destination = zones[1];
  std::replace(destination.begin(), destination.end(), WITHESPACESUBSTITUTE,
               ' ');
}

int ReadTimeStep(const std::string& input, const unsigned int week) {
  // input format should be x<timeStep>
  int timestep_in_problem = std::stoi(StringBetweenChevrons(input));
  // Two possible cases:
  // - either index is yearly (starts at (week-1)*167) => nothing to do
  // - or the index is weekly (starts at 0) => it needs to be shifted by +(week-1)*167
  if (timestep_in_problem < (week - 1) * 168) {
    timestep_in_problem += (week - 1) * 168;
  }
  return timestep_in_problem;
}

void updateMapColumn(const std::vector<ActiveLink>& links,
                     const std::string_view link_origin,
                     const std::string_view link_destination, colId id,
                     int time_step,
                     std::map<linkId, ColumnsToChange>& mapColumn) {
  auto it =
      std::find_if(links.begin(), links.end(),
                   [&link_origin, &link_destination](const ActiveLink& link) {
                     return link.linkor() == link_origin &&
                            link.linkex() == link_destination;
                   });

  if (it != links.end()) {
    mapColumn[it->get_idLink()].emplace_back(id, time_step);
  }
}

// VariableNameComposition VariableFieldsFromVariableName(
//     const std::string& var_name) {
//   auto vect_fields = StringManip::split(var_name, SEPARATOR);
//   VariableNameComposition result;
//   if (vect_fields.size() == 3) {
//     result.name = vect_fields[0];
//     ReadLinkZones(vect_fields[1], result.origin, result.destination);
//     result.time_step = ReadTimeStep(vect_fields[2]);
//   }

//   return result;
// }

void ProblemVariablesFromProblemAdapter::extract_variables(
    std::map<colId, ColumnsToChange>& p_ntc_columns,
    std::map<colId, ColumnsToChange>& p_direct_cost_columns,
    std::map<colId, ColumnsToChange>& p_indirect_cost_columns) const {
  // List of variables
  VariableFileReadNameConfiguration variable_name_config;
  variable_name_config.ntc_variable_name = "NTCDirect";
  variable_name_config.cost_origin_variable_name = "IntercoDirectCost";
  variable_name_config.cost_extremite_variable_name = "IntercoIndirectCost";

  const auto& var_names = problem_->get_col_names();
  std::string origin;
  std::string destination;
  for (size_t var_index = 0; var_index < var_names.size(); var_index++) {
    const auto& var_name = var_names.at(var_index);
    const auto& split_name = StringManip::split(var_name, SEPARATOR);

    if (split_name[0] == variable_name_config.ntc_variable_name) {
      ReadLinkZones(split_name[1], origin, destination);

      updateMapColumn(active_links_, origin, destination, var_index,
                      ReadTimeStep(split_name[2], problem_->Week()),
                      p_ntc_columns);
    } else if (split_name[0] ==
               variable_name_config.cost_origin_variable_name) {
      ReadLinkZones(split_name[1], origin, destination);
      updateMapColumn(active_links_, origin, destination, var_index,
                      ReadTimeStep(split_name[2], problem_->Week()),
                      p_direct_cost_columns);
    } else if (split_name[0] ==
               variable_name_config.cost_extremite_variable_name) {
      ReadLinkZones(split_name[1], origin, destination);
      updateMapColumn(active_links_, origin, destination, var_index,
                      ReadTimeStep(split_name[2], problem_->Week()),
                      p_indirect_cost_columns);
    }
  }
}

ProblemVariables ProblemVariablesFromProblemAdapter::Provide() {
  ProblemVariables result;
  extract_variables(result.ntc_columns, result.direct_cost_columns,
                    result.indirect_cost_columns);
  return result;
}

ProblemVariablesFromProblemAdapter::ProblemVariablesFromProblemAdapter(
    std::shared_ptr<Problem> problem, std::vector<ActiveLink> vector_1,
    std::shared_ptr<ProblemGenerationLog::ProblemGenerationLogger> shared_ptr_1)
    : problem_(std::move(problem)),
      active_links_(std::move(vector_1)),
      logger_(std::move(shared_ptr_1)) {}
