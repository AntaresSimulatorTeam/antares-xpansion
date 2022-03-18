#include "SensitivityLogger.h"

#include <iostream>

#include "Commons.h"

SensitivityLogger::SensitivityLogger(std::ostream& stream)
    : _stream(stream) {
  if (_stream.fail()) {
    std::cerr << "Invalid stream passed as parameter" << std::endl;
  }
}

void SensitivityLogger::display_message(const std::string& msg) {
  _stream << msg << std::endl;
}

void SensitivityLogger::log_at_start(
    const SensitivityOutputData& output_data) {
  _stream << std::endl;
  _stream << "Best overall cost = "
          << xpansion::logger::commons::create_str_million_euros(
                 output_data.best_benders_cost)
          << MILLON_EUROS << std::endl;
  _stream << "epsilon = " << output_data.epsilon << EUROS << std::endl;
  _stream << std::endl;
}

void SensitivityLogger::log_begin_pb_resolution(
    const SinglePbData& pb_data) {
  _stream << "Solving " << pb_data.get_pb_description() << "..." << std::endl;
}

std::string get_objective_unit(
    const SinglePbData& pb_data) {
  if (pb_data.pb_type == SensitivityPbType::PROJECTION) {
    return MW;
  } else {
    return MILLON_EUROS;
  }
}

std::string format_objective(
    const SinglePbData& pb_data) {
  if (pb_data.pb_type == SensitivityPbType::PROJECTION) {
    std::stringstream objective;
    objective << pb_data.objective;
    return objective.str();
  } else {
    return xpansion::logger::commons::create_str_million_euros(
        pb_data.objective);
  }
}

void SensitivityLogger::log_pb_solution(const SinglePbData& pb_data) {
  _stream << indent_1 << pb_data.get_pb_description() << " = "
          << format_objective(pb_data) << get_objective_unit(pb_data)
          << std::endl;
  _stream << indent_1 << "Overall cost = "
          << xpansion::logger::commons::create_str_million_euros(
                 pb_data.system_cost)
          << MILLON_EUROS << std::endl;
  _stream << std::endl;
}

void SensitivityLogger::log_at_ending() {
  _stream << "--- Sensitivity analysis completed ---" << std::endl;
}

void SensitivityLogger::log_summary(
    const SensitivityOutputData& output_data) {
  _stream << "Sensitivity analysis summary "
          << "(epsilon = " << output_data.epsilon << EUROS << "):" << std::endl
          << std::endl;

  std::vector<SinglePbData> projection_data = {};
  std::vector<SinglePbData> capex_data = {};

  for (const auto& pb_data : output_data.pbs_data) {
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
    log_projection_summary(projection_data);
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
          << MILLON_EUROS << ", "
          << xpansion::logger::commons::create_str_million_euros(capex_max)
          << MILLON_EUROS << "]" << std::endl
          << std::endl;
}

std::map<std::string, std::map<std::string, double>>
get_investment_intervals(
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
    const std::vector<SinglePbData>& projection_data) {
  auto investment_intervals = get_investment_intervals(projection_data);

  _stream << indent_1 << "Investment intervals of candidates:" << std::endl;
  for (const auto& kvp : investment_intervals) {
    auto interval = kvp.second;
    _stream << indent_1 << indent_1 << kvp.first << ": [" << interval[MIN_C]
            << MW << ", " << interval[MAX_C] << MW << "]" << std::endl;
  }
  _stream << std::endl;
}