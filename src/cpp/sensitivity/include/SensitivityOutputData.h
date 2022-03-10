#pragma once

#include <iostream>

#include "common.h"

const std::string MIN_C("min");
const std::string MAX_C("max");

enum class SensitivityPbType {
  CAPEX,
  PROJECTION,
};

struct SinglePbData {
  SensitivityPbType pb_type;
  std::string str_pb_type;
  std::string candidate_name;
  std::string opt_dir;
  double objective;
  double system_cost;
  Point candidates;
  int solver_status;

  SinglePbData() = default;
  SinglePbData(const SensitivityPbType &pb_type, const std::string &str_pb_type,
               const std::string &candidate_name, const std::string &opt_dir,
               double objective, double system_cost, const Point &candidates,
               int status)
      : pb_type(pb_type),
        str_pb_type(str_pb_type),
        candidate_name(candidate_name),
        opt_dir(opt_dir),
        objective(objective),
        system_cost(system_cost),
        candidates(candidates),
        solver_status(status) {}

  SinglePbData(const SensitivityPbType &pb_type, const std::string &str_pb_type,
               const std::string &candidate_name, const std::string &opt_dir)
      : pb_type(pb_type),
        str_pb_type(str_pb_type),
        candidate_name(candidate_name),
        opt_dir(opt_dir) {}

  std::string get_pb_description() const {
    return opt_dir + " " + str_pb_type + " " + candidate_name;
  };
};

struct SensitivityOutputData {
  double epsilon;
  double best_benders_cost;
  std::vector<SinglePbData> pbs_data;

  SensitivityOutputData() = default;
  SensitivityOutputData(double epsilon, double benders_cost,
                        const std::vector<SinglePbData> &pbs_data = {})
      : epsilon(epsilon), best_benders_cost(benders_cost), pbs_data(pbs_data) {}
};

struct RawPbData {
  int status;
  double obj_value;
  std::vector<double> obj_coeffs;
  std::vector<double> solution;
};