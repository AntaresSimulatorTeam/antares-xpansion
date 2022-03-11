#include "SensitivityFileLogger.h"

#include <iostream>
#include <string>

SensitivityFileLogger::SensitivityFileLogger(const std::string& filename) {
  _file.open(filename);
  if (_file.fail()) {
    std::cerr << "Invalid file name passed as parameter" << std::endl;
  }
  _userLog =
      std::unique_ptr<SensitivityLogger>(new SensitivityLogger(_file));
}

void SensitivityFileLogger::display_message(const std::string& msg) {
  _userLog->display_message(msg);
}

void SensitivityFileLogger::log_at_start(
    const SensitivityOutputData& output_data) {
  _userLog->log_at_start(output_data);
}

void SensitivityFileLogger::log_begin_pb_resolution(
    const SinglePbData& pb_data) {
  _userLog->log_begin_pb_resolution(pb_data);
}

void SensitivityFileLogger::log_pb_solution(const SinglePbData& pb_data) {
  _userLog->log_pb_solution(pb_data);
}

void SensitivityFileLogger::log_summary(
    const SensitivityOutputData& output_data) {
  _userLog->log_summary(output_data);
}

void SensitivityFileLogger::log_at_ending() { _userLog->log_at_ending(); }