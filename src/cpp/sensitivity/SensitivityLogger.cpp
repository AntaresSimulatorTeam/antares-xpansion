#include "SensitivityLogger.h"

#include <iostream>

#include "antares-xpansion/benders/logger/Commons.h"

SensitivityLogger::SensitivityLogger(std::ostream& stream) : _stream(stream) {
  if (_stream.fail()) {
    std::cerr << "Invalid stream passed as parameter" << std::endl;
  }
}

void SensitivityLogger::display_message(const std::string& msg) {
  _stream << msg << std::endl << std::endl;
}

void SensitivityLogger::log_at_start(const SensitivityInputData& input_data) {
  _stream << std::endl;
  _stream << "--- Recall of the best investment solution ---" << std::endl
          << std::endl;

  log_benders_overall_cost(input_data.best_ub);
  log_benders_capex(input_data.benders_capex);
  log_benders_solution(input_data.benders_solution);
  log_epsilon(input_data.epsilon);

  _stream << std::endl;
}

void SensitivityLogger::log_benders_overall_cost(const double& best_ub) {
  _stream << indent_1 << "Best overall cost = "
          << xpansion::logger::commons::create_str_million_euros(best_ub)
          << MILLON_EUROS << std::endl
          << std::endl;
}

void SensitivityLogger::log_benders_capex(const double& benders_capex) {
  _stream << indent_1 << "Best capex = "
          << xpansion::logger::commons::create_str_million_euros(benders_capex)
          << MILLON_EUROS << std::endl
          << std::endl;
}

void SensitivityLogger::log_benders_solution(
    const std::map<std::string, double>& benders_solution) {
  _stream << indent_1 << "Best investment solution = " << std::endl;
  for (const auto& [candidate_name, investment] : benders_solution) {
    _stream << indent_1 << indent_1 << candidate_name << ": "
            << xpansion::logger::commons::create_str_mw(investment) << MW
            << std::endl;
  }
}

void SensitivityLogger::log_epsilon(const double& epsilon) {
  _stream << std::endl;
  _stream << "--- Start sensitivity analysis with epsilon = " << epsilon
          << EUROS << " --- " << std::endl;
}

void SensitivityLogger::log_begin_pb_resolution(const SinglePbData& pb_data) {
  _stream << indent_1 << "Solving " << pb_data.get_pb_description() << "..."
          << std::endl;
}

std::string get_objective_unit(const SinglePbData& pb_data) {
  if (pb_data.pb_type == SensitivityPbType::PROJECTION) {
    return MW;
  } else {
    return MILLON_EUROS;
  }
}

std::string format_objective(const SinglePbData& pb_data) {
  std::string objective;
  if (pb_data.pb_type == SensitivityPbType::PROJECTION) {
    objective = xpansion::logger::commons::create_str_mw(pb_data.objective);
  } else {
    objective =
        xpansion::logger::commons::create_str_million_euros(pb_data.objective);
  }
  return objective;
}

void SensitivityLogger::log_pb_solution(const SinglePbData& pb_data) {
  _stream << indent_1 << indent_1 << pb_data.get_pb_description() << " = "
          << format_objective(pb_data) << get_objective_unit(pb_data)
          << std::endl;
  _stream << indent_1 << indent_1 << "Overall cost = "
          << xpansion::logger::commons::create_str_million_euros(
                 pb_data.system_cost)
          << MILLON_EUROS << std::endl;
  _stream << std::endl;
}

void SensitivityLogger::log_at_ending() {
  _stream << "--- Sensitivity study completed ---" << std::endl;
}

void SensitivityLogger::log_summary(const SensitivityInputData& input_data,
                                    const std::vector<SinglePbData>& pbs_data) {
  _stream << std::endl
          << "--- Sensitivity analysis summary "
          << "(epsilon = " << input_data.epsilon << EUROS << ") ---"
          << std::endl
          << std::endl;
  std::vector<SinglePbData> projection_data = {};
  std::vector<SinglePbData> capex_data = {};

  for (const auto& pb_data : pbs_data) {
    if (pb_data.pb_type == SensitivityPbType::PROJECTION) {
      projection_data.push_back(pb_data);
    } else {
      capex_data.push_back(pb_data);
    }
  }

  if (!capex_data.empty()) {
    log_capex_summary(capex_data);
  }
  if (!projection_data.empty()) {
    log_projection_summary(projection_data, input_data.candidates_bounds);
  }
}

void SensitivityLogger::log_capex_summary(
    const std::vector<SinglePbData>& capex_data) {
  double capex_min = 0;
  double capex_max = 0;

  for (const auto& pb_data : capex_data) {
    if (pb_data.opt_dir == MIN_C) {
      capex_min = pb_data.objective;
    } else if (pb_data.opt_dir == MAX_C) {
      capex_max = pb_data.objective;
    }
  }

  _stream << indent_1 << "CAPEX interval: ["
          << xpansion::logger::commons::create_str_million_euros(capex_min)
          << ", "
          << xpansion::logger::commons::create_str_million_euros(capex_max)
          << "]" << MILLON_EUROS << std::endl
          << std::endl;
}

std::map<std::string, std::map<std::string, double>> get_investment_intervals(
    const std::vector<SinglePbData>& projection_data) {
  std::map<std::string, std::map<std::string, double>> investment_intervals;

  for (const auto& pb_data : projection_data) {
    if (pb_data.opt_dir == MIN_C) {
      investment_intervals[pb_data.candidate_name][MIN_C] = pb_data.objective;
    } else {
      investment_intervals[pb_data.candidate_name][MAX_C] = pb_data.objective;
    }
  }
  return investment_intervals;
}

void SensitivityLogger::log_projection_summary(
    const std::vector<SinglePbData>& projection_data,
    const std::map<std::string, std::pair<double, double>>& candidates_bounds) {
  auto investment_intervals = get_investment_intervals(projection_data);

  _stream << indent_1 << "Investment intervals of candidates:" << std::endl;

  for (const auto& kvp : investment_intervals) {
    auto interval = kvp.second;

    _stream << indent_1 << indent_1 << kvp.first << ": ["
            << xpansion::logger::commons::create_str_mw(interval[MIN_C]) << ", "
            << xpansion::logger::commons::create_str_mw(interval[MAX_C]) << "]"
            << MW << indent_1 << "-- possible interval = ["
            << xpansion::logger::commons::create_str_mw(
                   candidates_bounds.at(kvp.first).first)
            << ", "
            << xpansion::logger::commons::create_str_mw(
                   candidates_bounds.at(kvp.first).second)
            << "]" << MW << std::endl;
  }
  _stream << std::endl;
}