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

void SensitivityUserLogger::log_at_start() {
  _stream << "--- Start sensitivity analysis ---" << std::endl;
}

void SensitivityUserLogger::log_set_sensitivity_pb(
    const SinglePbData& pb_data) {
  _stream << "Setting " << pb_data.get_pb_description() << " problem..."
          << std::endl;
}

void SensitivityUserLogger::log_begin_pb_resolution(
    const SinglePbData& pb_data) {
  _stream << "Solving " << pb_data.get_pb_description() << " problem..."
          << std::endl;
}

void SensitivityUserLogger::log_pb_solution(const SinglePbData& pb_data) {
  _stream << indent_1 << pb_data.get_pb_description() << " = "
          << pb_data.objective << std::endl;
  _stream << "Overall cost = "
          << xpansion::logger::commons::create_str_million_euros(
                 pb_data.system_cost)
          << " Me" << std::endl;
}

void SensitivityUserLogger::log_at_ending() {
  _stream << "--- Sensitivity analysis completed ---" << std::endl;
}