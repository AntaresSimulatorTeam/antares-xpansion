
#include "antares-xpansion/benders/benders_core/LastIterationReader.h"

#include <fstream>
#include <iostream>

#include "antares-xpansion/benders/benders_core/common.h"

LastIterationReader::LastIterationReader(
    const std::filesystem::path& last_iteration_file)
    : last_iteration_file_(last_iteration_file) {
  last_iteration_file_content_ = get_json_file_content(last_iteration_file_);
}
bool LastIterationReader::IsLastIterationFileValid() const {
  return !last_iteration_file_content_.isNull();
}
std::pair<LogData, LogData> LastIterationReader::LastIterationData() {
  return {GetIterationData("last"), GetIterationData("best_iteration")};
}
LogData LastIterationReader::GetIterationData(
    const std::string& iteration_name) {
  LogPoint x_cut;
  LogPoint min_invest;
  LogPoint max_invest;
  const auto candidates_array =
      last_iteration_file_content_[iteration_name]["candidates"];

  for (auto candidate = candidates_array.begin();
       candidate != candidates_array.end(); ++candidate) {
    x_cut.try_emplace((*candidate)["candidate"].asString(),
                   (*candidate)["invest"].asDouble());
    min_invest.try_emplace((*candidate)["candidate"].asString(),
                           (*candidate)["invest_min"].asDouble());
    max_invest.try_emplace((*candidate)["candidate"].asString(),
                           (*candidate)["invest_max"].asDouble());
  }
  return {
      last_iteration_file_content_[iteration_name]["lb"].asDouble(),
      last_iteration_file_content_[iteration_name]["best_ub"].asDouble(),
      last_iteration_file_content_[iteration_name]["ub"].asDouble(),
      last_iteration_file_content_[iteration_name]["it"].asInt(),
      last_iteration_file_content_[iteration_name]["best_it"].asInt(),
      last_iteration_file_content_[iteration_name]["subproblem_cost"]
          .asDouble(),
      last_iteration_file_content_[iteration_name]["invest_cost"].asDouble(),
      {},
      {},
      x_cut,
      min_invest,
      max_invest,
      last_iteration_file_content_[iteration_name]["optimality_gap"].asDouble(),
      last_iteration_file_content_[iteration_name]["relative_gap"].asDouble(),
      last_iteration_file_content_[iteration_name]["max_iterations"].asInt(),
      last_iteration_file_content_[iteration_name]["benders_elapsed_time"]
          .asDouble(),
      last_iteration_file_content_[iteration_name]["master_duration"]
          .asDouble(),
      last_iteration_file_content_[iteration_name]["subproblem_duration"]
          .asDouble(),
      last_iteration_file_content_[iteration_name]
                                  ["cumulative_number_of_subproblem_resolved"]
                                      .asInt()

  };
}
