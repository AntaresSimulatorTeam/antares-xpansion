#pragma once

#include <iostream>

#include "common.h"

enum class SensitivityPbType {
  CAPEX,
  PROJECTION,
};

struct SinglePbData {
  std::string pb_type;
  std::string opt_dir;
  double objective;
  double system_cost;
  Point candidates;
  int status;

  SinglePbData() = default;
  SinglePbData(const std::string &pb_type, const std::string &opt_dir,
               double objective, double system_cost, const Point &candidates,
               int status)
      : pb_type(pb_type),
        opt_dir(opt_dir),
        objective(objective),
        system_cost(system_cost),
        candidates(candidates),
        status(status) {}
  std::string get_pb_description() const {
    return opt_dir + " " + pb_type;
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