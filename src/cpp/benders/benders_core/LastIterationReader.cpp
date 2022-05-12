
#include "LastIterationReader.h"

#include <fstream>
#include <iostream>

LastIterationReader::LastIterationReader(
    const std::filesystem::path &last_iteration_file)
    : _last_iteration_file(last_iteration_file) {}

LogData LastIterationReader::last_iteration_data() const {
  auto last_iteration_file_content = _get_last_iteration_file_content();

  LogPoint x0;
  LogPoint min_invest;
  LogPoint max_invest;
  auto candidates_array = last_iteration_file_content["candidates"];

  for (Json::Value::const_iterator candidate = candidates_array.begin();
       candidate != candidates_array.end(); ++candidate) {
    x0.emplace((*candidate)["candidate"].asString(),
               (*candidate)["invest"].asDouble());
    min_invest.emplace((*candidate)["candidate"].asString(),
                       (*candidate)["invest_min"].asDouble());
    max_invest.emplace((*candidate)["candidate"].asString(),
                       (*candidate)["invest_max"].asDouble());
  }
  //   std::cout << "last_iteration_file_content[\"lb\"].asDouble()"
  //             << "= " << last_iteration_file_content["lb"].asDouble() <<
  //             std::endl
  //             << "last_iteration_file_content[\"best_ub\"].asDouble()"
  //             << "= " << last_iteration_file_content["best_ub"].asDouble()
  //             << std::endl
  //             << "last_iteration_file_content[\"it\"].asInt()"
  //             << "= " << last_iteration_file_content["it"].asInt() <<
  //             std::endl
  //             << "last_iteration_file_content[\"best_it\"].asInt()"
  //             << "= " << last_iteration_file_content["best_it"].asInt()
  //             << std::endl
  //             <<
  //             "last_iteration_file_content[\"subproblem_cost\"].asDouble()"
  //             << "= " <<
  //             last_iteration_file_content["subproblem_cost"].asDouble()
  //             << std::endl
  //             << "last_iteration_file_content[\"invest_cost\"].asDouble()= "
  //             << last_iteration_file_content["invest_cost"].asDouble()
  //             << std::endl;
  //   for (const auto &[candidate_name, value] : x0) {
  //     std::cout << "candidate" << candidate_name << " invest= " << value
  //               << std::endl
  //               << "invest_min= " << min_invest.at(candidate_name) <<
  //               std::endl
  //               << "invest_max= " << max_invest.at(candidate_name) <<
  //               std::endl;
  //   }
  return {last_iteration_file_content["lb"].asDouble(),
          last_iteration_file_content["best_ub"].asDouble(),
          last_iteration_file_content["it"].asInt(),
          last_iteration_file_content["best_it"].asInt(),
          last_iteration_file_content["subproblem_cost"].asDouble(),
          last_iteration_file_content["invest_cost"].asDouble(),
          x0,
          min_invest,
          max_invest,
          last_iteration_file_content["optimality_gap"].asDouble(),
          last_iteration_file_content["relative_gap"].asDouble(),
          last_iteration_file_content["max_iterations"].asDouble(),
          last_iteration_file_content["benders_elapsed_time"].asDouble()

  };
}
Json::Value LastIterationReader::_get_last_iteration_file_content() const {
  std::ifstream input_file_l(_last_iteration_file, std::ifstream::binary);

  Json::CharReaderBuilder builder_l;
  // json file content
  Json::Value file_content;
  std::string errs;
  if (!parseFromStream(builder_l, input_file_l, &file_content, &errs)) {
    std::cerr << errs << std::endl;
  }
  return file_content;
}