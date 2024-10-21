#include "antares-xpansion/benders/benders_core/LastIterationWriter.h"

#include <fstream>
#include <iostream>

void LastIterationWriter::SaveBestAndLastIterations(
    const LogData &best_iteration_log_data,
    const LogData &last_iteration_log_data) {
  best_iteration_data_ = best_iteration_log_data;
  last_iteration_data_ = last_iteration_log_data;
  FillOutput("last", last_iteration_data_);
  FillOutput("best_iteration", best_iteration_data_);
  std::ofstream jsonOut_l(output_file_);
  if (jsonOut_l) {
    // Output
    jsonOut_l << output_ << std::endl;
  } else {
    std::cerr << "Invalid Json file: " << output_file_.string() << std::endl;
  }
}

void LastIterationWriter::FillOutput(const std::string &iteration_name,
                                     const LogData &iteration_data) {
  output_[iteration_name]["lb"] = iteration_data.lb;
  output_[iteration_name]["ub"] = iteration_data.ub;
  output_[iteration_name]["best_ub"] = iteration_data.best_ub;
  output_[iteration_name]["it"] = iteration_data.it;
  output_[iteration_name]["best_it"] = iteration_data.best_it;
  output_[iteration_name]["subproblem_cost"] = iteration_data.subproblem_cost;
  output_[iteration_name]["invest_cost"] = iteration_data.invest_cost;

  Json::Value vectCandidates_l(Json::arrayValue);
  for (const auto &[candidate_name, value] : iteration_data.x_cut) {
    Json::Value candidate_l;
    candidate_l["candidate"] = candidate_name;
    candidate_l["invest"] = value;
    candidate_l["invest_min"] = iteration_data.min_invest.at(candidate_name);
    candidate_l["invest_max"] = iteration_data.max_invest.at(candidate_name);
    vectCandidates_l.append(candidate_l);
  }
  output_[iteration_name]["candidates"] = vectCandidates_l;
  output_[iteration_name]["optimality_gap"] = iteration_data.optimality_gap;
  output_[iteration_name]["relative_gap"] = iteration_data.relative_gap;
  output_[iteration_name]["max_iterations"] = iteration_data.max_iterations;
  output_[iteration_name]["benders_elapsed_time"] =
      iteration_data.benders_elapsed_time;
  output_[iteration_name]["master_duration"] = iteration_data.master_time;
  output_[iteration_name]["subproblem_duration"] =
      iteration_data.subproblem_time;
  output_[iteration_name]["cumulative_number_of_subproblem_resolved"] =
      iteration_data.cumulative_number_of_subproblem_resolved;
}