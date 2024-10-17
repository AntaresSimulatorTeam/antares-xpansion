#include "antares-xpansion/sensitivity/SensitivityWriter.h"

#include <fstream>
#include <iostream>
#include <utility>

#include "config.h"

SensitivityWriter::SensitivityWriter(std::filesystem::path json_filename)
    : _filename(std::move(json_filename)) {}

void dump(const Json::Value &output, const std::filesystem::path &file_name) {
  std::ofstream jsonOut_l(file_name);
  if (jsonOut_l) {
    jsonOut_l << output << std::endl;
  } else {
    std::cout << "Impossible d'ouvrir le fichier json " << file_name
              << std::endl;
  }
}

Json::Value write_candidate(const std::pair<std::string, double> &candidate) {
  Json::Value candidate_l;
  candidate_l[NAME_C] = candidate.first;
  candidate_l[INVEST_C] = candidate.second;
  return candidate_l;
}

Json::Value write_candidate_bounds(
    const std::pair<std::string, const std::pair<double, double>>
        &candidate_bounds) {
  Json::Value candidate_bounds_l;
  candidate_bounds_l[NAME_C] = candidate_bounds.first;
  candidate_bounds_l[LB_C] = candidate_bounds.second.first;
  candidate_bounds_l[UB_C] = candidate_bounds.second.second;
  return candidate_bounds_l;
}

Json::Value write_single_pb(const SinglePbData &single_pb_data) {
  Json::Value single_pb_data_l;

  std::string pb_description = single_pb_data.str_pb_type;
  if (single_pb_data.pb_type == SensitivityPbType::PROJECTION) {
    pb_description += " " + single_pb_data.candidate_name;
  }

  single_pb_data_l[PB_TYPE_C] = pb_description;
  single_pb_data_l[OPT_DIR_C] = single_pb_data.opt_dir;
  single_pb_data_l[STATUS_C] = single_pb_data.solver_status;
  single_pb_data_l[OBJECTIVE_C] = single_pb_data.objective;
  single_pb_data_l[SYSTEM_COST_C] = single_pb_data.system_cost;

  Json::Value candidates_l(Json::arrayValue);
  for (const auto &candidate : single_pb_data.candidates) {
    Json::Value candidate_l = write_candidate(candidate);
    candidates_l.append(candidate_l);
  }

  single_pb_data_l[CANDIDATES_C] = candidates_l;
  return single_pb_data_l;
}

Json::Value write_sensitivity_data(const SensitivityInputData &input_data,
                                   const std::vector<SinglePbData> &pbs_data) {
  Json::Value output;
  output[ANTARES_C][VERSION_C] = ANTARES_VERSION_TAG;
  output[ANTARES_XPANSION_C][VERSION_C] = PROJECT_VER;

  output[EPSILON_C] = input_data.epsilon;
  output[BEST_BENDERS_C] = input_data.best_ub;

  Json::Value candidates_bounds_l(Json::arrayValue);
  Json::Value pbs_data_l(Json::arrayValue);

  for (const auto &candidate_bounds : input_data.candidates_bounds) {
    Json::Value candidate_bounds_l = write_candidate_bounds(candidate_bounds);
    candidates_bounds_l.append(candidate_bounds_l);
  }

  for (const auto &single_pb_data : pbs_data) {
    Json::Value single_pb_data_l = write_single_pb(single_pb_data);
    pbs_data_l.append(single_pb_data_l);
  }

  output[BOUNDS_C] = candidates_bounds_l;
  output[SENSITIVITY_SOLUTION_C] = pbs_data_l;
  return output;
}

void SensitivityWriter::end_writing(
    const SensitivityInputData &input_data,
    const std::vector<SinglePbData> &pbs_data) const {
  auto output = write_sensitivity_data(input_data, pbs_data);
  dump(output, _filename);
}