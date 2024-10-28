#pragma once

#include <iostream>
#include <utility>

#include "antares-xpansion/benders/benders_core/common.h"

const std::string MIN_C("min");
const std::string MAX_C("max");
const std::string PROJECTION_C("investment");
const std::string CAPEX_C("capex");

enum class SensitivityPbType {
  CAPEX,
  PROJECTION,
};

inline std::string get_string_from_SensitivityPbType(SensitivityPbType type) {
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
    std::string pb_description = opt_dir + " " + str_pb_type;
    if (pb_type == SensitivityPbType::PROJECTION) {
      pb_description += " " + candidate_name;
    }
    return pb_description;
  };
};

struct RawPbData {
  int status;
  double obj_value;
  std::vector<double> obj_coeffs;
  std::vector<double> solution;
};