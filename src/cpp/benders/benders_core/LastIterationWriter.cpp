#include "LastIterationWriter.h"

#include <fstream>
#include <iostream>

void LastIterationWriter::save_best_and_last_iterations(
    const LogData &best_iteration_log_data,
    const LogData &last_iteration_log_data) {
  _best_iteration_data = best_iteration_log_data;
  _last_iteration_data = last_iteration_log_data;
  _fill_output("last", _last_iteration_data);
  _fill_output("best_iteration", _best_iteration_data);
  std::ofstream jsonOut_l(_output_file);
  if (jsonOut_l) {
    // Output
    jsonOut_l << _output << std::endl;
  } else {
    std::cerr << "Invalid Json file: " << _output_file.string() << std::endl;
  }
}

void LastIterationWriter::_fill_output(const std::string &iteration_name,
                                       const LogData &iteration_data) {
  _output[iteration_name]["lb"] = iteration_data.lb;
  _output[iteration_name]["ub"] = iteration_data.ub;
  _output[iteration_name]["best_ub"] = iteration_data.best_ub;
  _output[iteration_name]["it"] = iteration_data.it;
  _output[iteration_name]["best_it"] = iteration_data.best_it;
  _output[iteration_name]["subproblem_cost"] = iteration_data.subproblem_cost;
  _output[iteration_name]["invest_cost"] = iteration_data.invest_cost;

  Json::Value vectCandidates_l(Json::arrayValue);
  for (const auto &[candidate_name, value] : iteration_data.x0) {
    Json::Value candidate_l;
    candidate_l["candidate"] = candidate_name;
    candidate_l["invest"] = value;
    candidate_l["invest_min"] = iteration_data.min_invest.at(candidate_name);
    candidate_l["invest_max"] = iteration_data.max_invest.at(candidate_name);
    vectCandidates_l.append(candidate_l);
  }
  _output[iteration_name]["candidates"] = vectCandidates_l;
  _output[iteration_name]["optimality_gap"] = iteration_data.optimality_gap;
  _output[iteration_name]["relative_gap"] = iteration_data.relative_gap;
  _output[iteration_name]["max_iterations"] = iteration_data.max_iterations;
  _output[iteration_name]["benders_elapsed_time"] =
      iteration_data.benders_elapsed_time;
  _output[iteration_name]["duration"] = iteration_data.master_time;
}