#pragma once

#include <json/json.h>
#include "antares-xpansion/multisolver_interface/SolverAbstract.h"

#include <iostream>

struct SensitivityInputData {
  double epsilon;
  double best_ub;
  double benders_capex;
  std::map<std::string, double> benders_solution;
  std::map<std::string, int> name_to_id;
  std::shared_ptr<SolverAbstract> last_master;
  std::filesystem::path basis_file_path;
  std::map<std::string, std::pair<double, double>> candidates_bounds;
  bool capex;
  std::vector<std::string> projection;
};

class SensitivityInputReader {
 public:
  SensitivityInputReader() = default;
  explicit SensitivityInputReader(
      const std::filesystem::path &json_input_path,
      const std::filesystem::path &benders_output_path,
      std::filesystem::path last_master_path, std::filesystem::path basis_path,
      std::filesystem::path structure_path);
  ~SensitivityInputReader() = default;

  SensitivityInputData get_input_data() const;

 private:
  Json::Value _json_data;
  Json::Value _benders_data;
  std::filesystem::path _last_master_path;
  std::filesystem::path _basis_file_path;
  std::filesystem::path _structure_file_path;

  std::shared_ptr<SolverAbstract> get_last_master() const;
  double get_best_ub() const;
  double get_benders_capex() const;
  std::map<std::string, double> get_benders_solution() const;
  std::map<std::string, int> get_name_to_id() const;
  std::vector<std::string> get_projection() const;
  std::map<std::string, std::pair<double, double>> get_candidates_bounds(
      SolverAbstract *last_master,
      const std::map<std::string, int> &name_to_id) const;
};
