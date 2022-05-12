#include "LastIterationRecorder.h"

#include <fstream>
#include <iostream>

void LastIterationRecorder::save_last_iteration(
    const LogData &iteration_log_data) {
  _last_iteration_data = iteration_log_data;
  _fill_output();
  std::ofstream jsonOut_l(_output_file);
  if (jsonOut_l) {
    // Output
    jsonOut_l << _output << std::endl;
  } else {
    std::cerr << "Impossible d'ouvrir le fichier json " << _output_file
              << std::endl;
  }
}

void LastIterationRecorder::_fill_output() {
  _output["lb"] = _last_iteration_data.lb;
  _output["best_ub"] = _last_iteration_data.best_ub;
  _output["it"] = _last_iteration_data.it;
  _output["best_it"] = _last_iteration_data.best_it;
  _output["subproblem_cost"] = _last_iteration_data.subproblem_cost;
  _output["invest_cost"] = _last_iteration_data.invest_cost;

  Json::Value vectCandidates_l(Json::arrayValue);
  for (const auto &[candidate_name, value] : _last_iteration_data.x0) {
    Json::Value candidate_l;
    candidate_l["candidate"] = candidate_name;
    candidate_l["invest"] = value;
    candidate_l["invest_min"] =
        _last_iteration_data.min_invest.at(candidate_name);
    candidate_l["invest_max"] =
        _last_iteration_data.max_invest.at(candidate_name);
    vectCandidates_l.append(candidate_l);
  }
  _output["candidates"] = vectCandidates_l;
  _output["optimality_gap"] = _last_iteration_data.optimality_gap;
  _output["relative_gap"] = _last_iteration_data.relative_gap;
  _output["max_iterations"] = _last_iteration_data.max_iterations;
  _output["benders_elapsed_time"] = _last_iteration_data.benders_elapsed_time;
}