
#include "LastIterationReader.h"

#include <fstream>
#include <iostream>

#include "common.h"

LastIterationReader::LastIterationReader(
    const std::filesystem::path& last_iteration_file)
    : _last_iteration_file(last_iteration_file) {
  _last_iteration_file_content = get_json_file_content(_last_iteration_file);
}
bool LastIterationReader::is_last_iteration_file_valid() const {
  return _last_iteration_file_content != Json::Value::null;
}
std::pair<LogData, LogData> LastIterationReader::last_iteration_data() {
  return {_get_iteration_data("last"), _get_iteration_data("best_iteration")};
}
LogData LastIterationReader::_get_iteration_data(
    const std::string& iteration_name) {
  LogPoint x0;
  LogPoint min_invest;
  LogPoint max_invest;
  const auto candidates_array =
      _last_iteration_file_content[iteration_name]["candidates"];

  for (auto candidate = candidates_array.begin();
       candidate != candidates_array.end(); ++candidate) {
    x0.emplace((*candidate)["candidate"].asString(),
               (*candidate)["invest"].asDouble());
    min_invest.emplace((*candidate)["candidate"].asString(),
                       (*candidate)["invest_min"].asDouble());
    max_invest.emplace((*candidate)["candidate"].asString(),
                       (*candidate)["invest_max"].asDouble());
  }
  return {
      _last_iteration_file_content[iteration_name]["lb"].asDouble(),
      _last_iteration_file_content[iteration_name]["ub"].asDouble(),
      _last_iteration_file_content[iteration_name]["best_ub"].asDouble(),
      _last_iteration_file_content[iteration_name]["it"].asInt(),
      _last_iteration_file_content[iteration_name]["best_it"].asInt(),
      _last_iteration_file_content[iteration_name]["subproblem_cost"]
          .asDouble(),
      _last_iteration_file_content[iteration_name]["invest_cost"].asDouble(),
      x0,
      min_invest,
      max_invest,
      _last_iteration_file_content[iteration_name]["optimality_gap"].asDouble(),
      _last_iteration_file_content[iteration_name]["relative_gap"].asDouble(),
      _last_iteration_file_content[iteration_name]["max_iterations"].asInt(),
      _last_iteration_file_content[iteration_name]["benders_elapsed_time"]
          .asDouble(),
      _last_iteration_file_content[iteration_name]["duration"].asDouble()

  };
}
