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
  std::string opt_dir;
  double objective;
  double system_cost;
  Point candidates;
  int status;

  SinglePbData() = default;
  SinglePbData(const std::string &str_pb_type, const std::string &opt_dir,
               double objective, double system_cost, const Point &candidates,
               int status)
      : str_pb_type(str_pb_type),
        opt_dir(opt_dir),
        objective(objective),
        system_cost(system_cost),
        candidates(candidates),
        status(status) {}

  SinglePbData(const SensitivityPbType &pb_type, const std::string &str_pb_type,
               const std::string &opt_dir)
      : pb_type(pb_type), str_pb_type(str_pb_type), opt_dir(opt_dir) {}

  std::string get_pb_description() const {
    return opt_dir + " " + str_pb_type;
  };

  bool is_capex() const { return pb_type == SensitivityPbType::CAPEX; }
  bool is_capex_min() const { return is_capex() && (opt_dir == MIN_C); }
  bool is_capex_max() const { return is_capex() && (opt_dir == MAX_C); }
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