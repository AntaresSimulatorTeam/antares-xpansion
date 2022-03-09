#include "SensitivityMasterLogger.h"

void SensitivityMasterLogger::display_message(const std::string &msg) {
  for (auto logger : _loggers) {
    logger->display_message(msg);
  }
}

void SensitivityMasterLogger::log_at_start(const SensitivityOutputData &output_data) {
  for (auto logger : _loggers) {
    logger->log_at_start(output_data);
  }
}

void SensitivityMasterLogger::log_begin_pb_resolution(
    const SinglePbData &pb_data) {
  for (auto logger : _loggers) {
    logger->log_begin_pb_resolution(pb_data);
  }
}

void SensitivityMasterLogger::log_pb_solution(const SinglePbData &pb_data) {
  for (auto logger : _loggers) {
    logger->log_pb_solution(pb_data);
  }
}

void SensitivityMasterLogger::log_summary(const SensitivityOutputData &output_data) {
  for (auto logger : _loggers) {
    logger->log_summary(output_data);
  }
}

void SensitivityMasterLogger::log_at_ending() {
  for (auto logger : _loggers) {
    logger->log_at_ending();
  }
}