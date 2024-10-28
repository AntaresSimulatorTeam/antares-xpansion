#include "antares-xpansion/sensitivity/SensitivityMasterLogger.h"

void SensitivityMasterLogger::display_message(const std::string& msg) {
  for (const auto& logger : _loggers) {
    logger->display_message(msg);
  }
}

void SensitivityMasterLogger::log_at_start(
    const SensitivityInputData& input_data) {
  for (const auto& logger : _loggers) {
    logger->log_at_start(input_data);
  }
}

void SensitivityMasterLogger::log_begin_pb_resolution(
    const SinglePbData& pb_data) {
  for (const auto& logger : _loggers) {
    logger->log_begin_pb_resolution(pb_data);
  }
}

void SensitivityMasterLogger::log_pb_solution(const SinglePbData& pb_data) {
  for (const auto& logger : _loggers) {
    logger->log_pb_solution(pb_data);
  }
}

void SensitivityMasterLogger::log_summary(
    const SensitivityInputData& input_data,
    const std::vector<SinglePbData>& pbs_data) {
  for (const auto& logger : _loggers) {
    logger->log_summary(input_data, pbs_data);
  }
}

void SensitivityMasterLogger::log_at_ending() {
  for (const auto& logger : _loggers) {
    logger->log_at_ending();
  }
}