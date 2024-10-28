#include "antares-xpansion/sensitivity/SensitivityFileLogger.h"

#include <iostream>
#include <memory>
#include <string>

SensitivityFileLogger::SensitivityFileLogger(
    const std::filesystem::path& filename) {
  _file.open(filename);
  if (_file.fail()) {
    std::cerr << "Invalid file name passed as parameter" << std::endl;
  }
  _userLog = std::make_unique<SensitivityLogger>(SensitivityLogger(_file));
}

void SensitivityFileLogger::display_message(const std::string& msg) {
  _userLog->display_message(msg);
}

void SensitivityFileLogger::log_at_start(
    const SensitivityInputData& input_data) {
  _userLog->log_at_start(input_data);
}

void SensitivityFileLogger::log_begin_pb_resolution(
    const SinglePbData& pb_data) {
  _userLog->log_begin_pb_resolution(pb_data);
}

void SensitivityFileLogger::log_pb_solution(const SinglePbData& pb_data) {
  _userLog->log_pb_solution(pb_data);
}

void SensitivityFileLogger::log_summary(
    const SensitivityInputData& input_data,
    const std::vector<SinglePbData>& pbs_data) {
  _userLog->log_summary(input_data, pbs_data);
}

void SensitivityFileLogger::log_at_ending() { _userLog->log_at_ending(); }