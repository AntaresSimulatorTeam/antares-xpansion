#include "antares-xpansion/sensitivity/SensitivityInputReader.h"

#include <boost/algorithm/string/trim.hpp>
#include <fstream>
#include <utility>

#include "LogUtils.h"
#include "OutputWriter.h"
#include "antares-xpansion/benders/benders_core/common.h"
#include "antares-xpansion/multisolver_interface/SolverFactory.h"

const std::string EPSILON_C("epsilon");
const std::string CAPEX_C("capex");
const std::string PROJECTION_C("projection");

Json::Value read_json(const std::filesystem::path &json_file_path) {
  std::ifstream json_file(json_file_path);
  Json::Value json_data;
  if (json_file.good()) {
    json_file >> json_data;
  } else {
    throw std::runtime_error(LOGLOCATION +
                             "unable to open : " + json_file_path.string());
  }
  return json_data;
}

SensitivityInputReader::SensitivityInputReader(
    const std::filesystem::path &json_input_path,
    const std::filesystem::path &benders_output_path,
    std::filesystem::path last_master_path, std::filesystem::path basis_path,
    std::filesystem::path structure_path)
    : _last_master_path(std::move(last_master_path)),
      _basis_file_path(std::move(basis_path)),
      _structure_file_path(std::move(structure_path)) {
  _json_data = read_json(json_input_path);
  _benders_data = read_json(benders_output_path);
}

std::vector<std::string> SensitivityInputReader::get_projection() const {
  std::vector<std::string> projection;
  for (const auto &candidate_name : _json_data[PROJECTION_C]) {
    projection.push_back(candidate_name.asString());
  }
  return projection;
}

SolverAbstract::Ptr SensitivityInputReader::get_last_master() const {
  SolverFactory factory;
  SolverAbstract::Ptr last_master;

  if (_benders_data[Output::OPTIONS_C]["SOLVER_NAME"].asString() == "COIN") {
    last_master = factory.create_solver("CBC");
  } else {
    last_master = factory.create_solver(
        _benders_data[Output::OPTIONS_C]["SOLVER_NAME"].asString());
  }
  last_master->set_threads(1);
  last_master->set_output_log_level(
      _benders_data[Output::OPTIONS_C]["LOG_LEVEL"].asInt());
  last_master->read_prob_mps(_last_master_path);
  return last_master;
}

std::map<std::string, std::pair<double, double>>
SensitivityInputReader::get_candidates_bounds(
    SolverAbstract::Ptr last_master,
    const std::map<std::string, int> &name_to_id) const {
  std::map<std::string, std::pair<double, double>> candidates_bounds;

  unsigned int nb_candidates = name_to_id.size();
  std::vector<double> lb_buffer(nb_candidates);
  std::vector<double> ub_buffer(nb_candidates);
  std::vector<std::string> candidates_name =
      last_master->get_col_names(0, nb_candidates - 1);

  for (auto &cand_name : candidates_name) {
    boost::algorithm::trim_right(cand_name);
  }

  last_master->get_lb(lb_buffer.data(), 0, nb_candidates - 1);
  last_master->get_ub(ub_buffer.data(), 0, nb_candidates - 1);

  for (int i = 0; i < nb_candidates; i++) {
    candidates_bounds[candidates_name[i]] = {lb_buffer[i], ub_buffer[i]};
  }
  return candidates_bounds;
}

double SensitivityInputReader::get_best_ub() const {
  return _benders_data[Output::SOLUTION_C][Output::OVERALL_COST_C].asDouble();
}

double SensitivityInputReader::get_benders_capex() const {
  return _benders_data[Output::SOLUTION_C][Output::INVESTMENT_COST_C]
      .asDouble();
}

std::map<std::string, double> SensitivityInputReader::get_benders_solution()
    const {
  std::map<std::string, double> investment_solution = {};
  for (const auto &candidate_name :
       _benders_data[Output::SOLUTION_C][Output::VALUES_C].getMemberNames()) {
    investment_solution[candidate_name] =
        _benders_data[Output::SOLUTION_C][Output::VALUES_C][candidate_name]
            .asDouble();
  }
  return investment_solution;
}

std::map<std::string, int> SensitivityInputReader::get_name_to_id() const {
  std::map<std::string, int> name_to_id;
  std::ifstream structure(_structure_file_path);
  if (structure.good()) {
    std::string line;

    while (std::getline(structure, line)) {
      std::stringstream buffer(line);
      std::string problem_name;
      std::string variable_name;
      int variable_id;
      buffer >> problem_name;
      buffer >> variable_name;
      buffer >> variable_id;

      if (problem_name ==
          _benders_data[Output::OPTIONS_C]["MASTER_NAME"].asString()) {
        name_to_id[variable_name] = variable_id;
      }
    }
  } else {
    throw std::runtime_error(
        LOGLOCATION + "unable to open : " + _structure_file_path.string());
  }
  return name_to_id;
}

SensitivityInputData SensitivityInputReader::get_input_data() const {
  SensitivityInputData input_data;
  input_data.epsilon = _json_data[EPSILON_C].asDouble();
  input_data.best_ub = get_best_ub();
  input_data.benders_capex = get_benders_capex();
  input_data.benders_solution = get_benders_solution();
  input_data.name_to_id = get_name_to_id();
  input_data.last_master = get_last_master();
  input_data.basis_file_path = _basis_file_path;
  input_data.candidates_bounds =
      get_candidates_bounds(input_data.last_master, input_data.name_to_id);
  input_data.capex = _json_data[CAPEX_C].asBool();
  input_data.projection = get_projection();
  return input_data;
}