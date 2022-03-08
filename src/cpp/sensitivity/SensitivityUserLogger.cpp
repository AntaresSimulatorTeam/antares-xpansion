#include "SensitivityUserLogger.h"

#include <iostream>

#include "Commons.h"

const std::string indent_1 = "\t";

SensitivityUserLogger::SensitivityUserLogger(std::ostream& stream)
    : _stream(stream) {
  if (_stream.fail()) {
    std::cerr << "Invalid stream passed as parameter" << std::endl;
  }
}

void SensitivityUserLogger::display_message(const std::string& msg) {
  _stream << msg << std::endl;
}

void SensitivityUserLogger::log_at_start(
    const SensitivityOutputData& output_data) {
  _stream << std::endl;
  _stream << "Best overall cost = "
          << xpansion::logger::commons::create_str_million_euros(
                 output_data.best_benders_cost)
          << " Me" << std::endl;
  _stream << "epsilon = " << output_data.epsilon << std::endl;
  _stream << std::endl;
}

void SensitivityUserLogger::log_set_sensitivity_pb(
    const SinglePbData& pb_data) {
  _stream << "Setting " << pb_data.get_pb_description() << std::endl;
}

void SensitivityUserLogger::log_begin_pb_resolution(
    const SinglePbData& pb_data) {
  _stream << "Solving " << pb_data.get_pb_description() << "..." << std::endl;
}

void SensitivityUserLogger::log_pb_solution(const SinglePbData& pb_data) {
  _stream << indent_1 << pb_data.get_pb_description() << " = "
          << pb_data.objective << " MW" << std::endl;
  _stream << indent_1 << "Overall cost = "
          << xpansion::logger::commons::create_str_million_euros(
                 pb_data.system_cost)
          << " Me" << std::endl;
  _stream << std::endl;
}

void SensitivityUserLogger::log_at_ending() {
  _stream << "--- Sensitivity analysis completed ---" << std::endl;
}

void SensitivityUserLogger::log_summary(
    const SensitivityOutputData& output_data) {
  _stream << "Sensitivity analysis summary:" << std::endl;
  log_capex_summary(output_data);
}

void SensitivityUserLogger::log_capex_summary(
    const SensitivityOutputData& output_data) {
  double capex_min = 0;
  double capex_max = 0;
  auto capex_min_pb_it = std::find_if(output_data.pbs_data.begin(),
                                      output_data.pbs_data.end(), is_capex_min);
  auto capex_max_pb_it = std::find_if(output_data.pbs_data.begin(),
                                      output_data.pbs_data.end(), is_capex_max);
  if (capex_min_pb_it != output_data.pbs_data.end()) {
    capex_min = (*capex_min_pb_it).objective;
  } else {
    display_message("CAPEX min value not found");
  }
  if (capex_max_pb_it != output_data.pbs_data.end()) {
    capex_max = (*capex_max_pb_it).objective;
  } else {
    display_message("CAPEX max value not found");
  }

  _stream << indent_1 << "CAPEX interval: ["
          << xpansion::logger::commons::create_str_million_euros(capex_min)
          << " Me, "
          << xpansion::logger::commons::create_str_million_euros(capex_max)
          << " Me]" << std::endl;
}

bool SensitivityUserLogger::is_capex_min(const SinglePbData& pb_data) {
  return pb_data.is_capex_min();
}

bool SensitivityUserLogger::is_capex_max(const SinglePbData& pb_data) {
  return pb_data.is_capex_max();
}