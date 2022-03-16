#pragma once

#include <iostream>
#include <utility>

#include "common.h"

const std::string MIN_C("min");
const std::string MAX_C("max");
const std::string PROJECTION_C("investment");
const std::string CAPEX_C("capex");

enum class SensitivityPbType {
  CAPEX,
  PROJECTION,
};

std::string get_string_from_SensitivityPbType(SensitivityPbType type) {
  if (type == SensitivityPbType::PROJECTION) {
    return PROJECTION_C;
  } else if (type == SensitivityPbType::CAPEX) {
    return CAPEX_C;
  } else {
    return "";
  }
}
struct SinglePbData {
  SensitivityPbType pb_type = SensitivityPbType::CAPEX;
  std::string str_pb_type;
  std::string candidate_name;
  std::string opt_dir;
  double objective = 0;
  double system_cost = 0;
  Point candidates;
  int solver_status = 0;

  SinglePbData() = default;
  SinglePbData(const SensitivityPbType &pb_type, std::string str_pb_type,
               std::string candidate_name, std::string opt_dir,
               double objective, double system_cost, Point candidates,
               int status)
      : pb_type(pb_type),
        str_pb_type(std::move(str_pb_type)),
        candidate_name(std::move(candidate_name)),
        opt_dir(std::move(opt_dir)),
        objective(objective),
        system_cost(system_cost),
        candidates(std::move(candidates)),
        solver_status(status) {}

  SinglePbData(const SensitivityPbType &pb_type, std::string str_pb_type,
               std::string candidate_name, std::string opt_dir)
      : pb_type(pb_type),
        str_pb_type(std::move(str_pb_type)),
        candidate_name(std::move(candidate_name)),
        opt_dir(std::move(opt_dir)) {}

  std::string get_pb_description() const {
    return opt_dir + " " + str_pb_type + " " + candidate_name;
  };
};

struct SensitivityOutputData {
  double epsilon = 0;
  double best_benders_cost = 0;
  std::vector<SinglePbData> pbs_data;

  SensitivityOutputData() = default;
  SensitivityOutputData(double epsilon, double benders_cost,
                        std::vector<SinglePbData> pbs_data = {})
      : epsilon(epsilon),
        best_benders_cost(benders_cost),
        pbs_data(std::move(pbs_data)) {}
};

struct RawPbData {
  int status;
  double obj_value;
  std::vector<double> obj_coeffs;
  std::vector<double> solution;
};