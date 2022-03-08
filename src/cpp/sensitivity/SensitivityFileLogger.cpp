#include "SensitivityFileLogger.h"

#include <string>
#include <iostream>

SensitivityFileLogger::SensitivityFileLogger(const std::string& filename) {
  _file.open(filename);
  if (_file.fail()) {
    std::cerr << "Invalid file name passed as parameter" << std::endl;
  }
  _userLog =
      std::unique_ptr<SensitivityUserLogger>(new SensitivityUserLogger(_file));
}

SensitivityFileLogger::~SensitivityFileLogger() { _file.close(); }

void SensitivityFileLogger::display_message(const std::string& msg) {
  _userLog->display_message(msg);
}

void SensitivityFileLogger::log_at_start() { _userLog->log_at_start(); }

void SensitivityFileLogger::log_set_sensitivity_pb(
    const SinglePbData& pb_data) {
  _userLog->log_set_sensitivity_pb(pb_data);
}

void SensitivityFileLogger::log_begin_pb_resolution(
    const SinglePbData& pb_data) {
  _userLog->log_begin_pb_resolution(pb_data);
}

void SensitivityFileLogger::log_pb_solution(const SinglePbData& pb_data) {
  _userLog->log_pb_solution(pb_data);
}

void SensitivityFileLogger::log_at_ending() { _userLog->log_at_ending(); }