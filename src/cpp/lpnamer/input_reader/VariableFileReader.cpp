//
#include "VariableFileReader.h"

#include <algorithm>
#include <sstream>

void updateMapColumn(const std::vector<ActiveLink>& links, int link_id,
                     colId id, int time_step,
                     std::map<linkId, ColumnsToChange>& mapColumn) {
  auto it = std::find_if(links.begin(), links.end(),
                         [link_id](const ActiveLink& link) {
                           return link.get_idLink() == link_id;
                         });

  if (it != links.end()) {
    mapColumn[link_id].push_back({id, time_step});
  }
}

VariableFileReader::VariableFileReader(
    const std::filesystem::path& path, const std::vector<ActiveLink>& links,
    const VariableFileReadNameConfiguration& variable_name_config,
    ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger)
    : logger_(logger) {
  std::string line;
  std::ifstream file(path);
  if (!file.good()) {
    auto errMsg = std::string("Unable to open '") + path.string() + "'";
    (*logger_)(ProblemGenerationLog::LOGLEVEL::FATAL) << errMsg;
    throw std::runtime_error(errMsg);
  }
  ReadVarsFromStream(file, links, variable_name_config);
  file.close();
}

VariableFileReader::VariableFileReader(
    std::istringstream& fileInIStringStream,
    const std::vector<ActiveLink>& links,
    const VariableFileReadNameConfiguration& variable_name_config,
    ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger)
    : logger_(logger) {
  ReadVarsFromStream(fileInIStringStream, links, variable_name_config);
}
void VariableFileReader::ReadVarsFromStream(
    std::istream& stream, const std::vector<ActiveLink>& links,
    const VariableFileReadNameConfiguration& variable_name_config) {
  std::string line;
  while (std::getline(stream, line)) {
    std::string name = getVarNameFromLine(line);
    _variables.push_back(name);

    std::istringstream buffer(line);
    colId id;
    std::string type_de_variable;
    buffer >> id;
    buffer >> type_de_variable;

    if (type_de_variable == variable_name_config.ntc_variable_name) {
      int pays;
      int link_id;
      int time_step;
      buffer >> pays;
      buffer >> link_id;
      buffer >> time_step;
      updateMapColumn(links, link_id, id, time_step, _ntc_p_var_columns);
    } else if (type_de_variable ==
               variable_name_config.cost_extremite_variable_name) {
      int link_id;
      int time_step;
      buffer >> link_id;
      buffer >> time_step;
      updateMapColumn(links, link_id, id, time_step,
                      _indirect_cost_p_var_columns);
    } else if (type_de_variable ==
               variable_name_config.cost_origin_variable_name) {
      int link_id;
      int time_step;
      buffer >> link_id;
      buffer >> time_step;
      updateMapColumn(links, link_id, id, time_step,
                      _direct_cost_p_var_columns);
    }
  }
}
std::string VariableFileReader::getVarNameFromLine(
    const std::string& line) const {
  std::ostringstream name;
  {
    std::istringstream buffer(line);
    std::string part;
    bool is_first(true);
    while (buffer >> part) {
      if (!is_first) {
        name << part << "_";
      } else {
        is_first = false;
      }
    }
  }
  return name.str();
}

const std::vector<std::string>& VariableFileReader::getVariables() const {
  return _variables;
}

const std::map<linkId, ColumnsToChange>& VariableFileReader::getNtcVarColumns()
    const {
  return _ntc_p_var_columns;
}

const std::map<linkId, ColumnsToChange>&
VariableFileReader::getDirectCostVarColumns() const {
  return _direct_cost_p_var_columns;
}

const std::map<linkId, ColumnsToChange>&
VariableFileReader::getIndirectCostVarColumns() const {
  return _indirect_cost_p_var_columns;
}
