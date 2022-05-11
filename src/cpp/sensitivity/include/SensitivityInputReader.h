#pragma once

#include <json/json.h>
#include <multisolver_interface/SolverAbstract.h>

#include <iostream>

struct SensitivityInputData {
  double epsilon;
  double best_ub;
  std::map<std::string, int> name_to_id;
  SolverAbstract::Ptr last_master;
  std::string basis_file_path;
  std::map<std::string, std::pair<double, double>> candidates_bounds;
  bool capex;
  std::vector<std::string> projection;
};

class SensitivityInputReader {
 public:
  SensitivityInputReader() = default;
  explicit SensitivityInputReader(const std::string &json_input_path,
                                  const std::string &benders_output_path,
                                  std::string last_master_path,
                                  std::string basis_path,
                                  std::string structure_path);
  ~SensitivityInputReader() = default;

  SensitivityInputData get_input_data() const;

 private:
  Json::Value _json_data;
  Json::Value _benders_data;
  std::string _last_master_path;
  std::string _basis_file_path;
  std::string _structure_file_path;

  SolverAbstract::Ptr get_last_master() const;
  double get_best_ub() const;
  std::map<std::string, int> get_name_to_id() const;
  std::vector<std::string> get_projection() const;
  std::map<std::string, std::pair<double, double>> get_candidates_bounds(
      SolverAbstract::Ptr last_master,
      const std::map<std::string, int> &name_to_id) const;
};
