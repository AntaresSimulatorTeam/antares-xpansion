#include "SensitivityMasterLogger.h"

void SensitivityMasterLogger::display_message(const std::string &msg) {
  for (auto logger : _loggers) {
    logger->display_message(msg);
  }
}

void SensitivityMasterLogger::log_at_start() {
  for (auto logger : _loggers) {
    logger->log_at_start();
  }
}

void SensitivityMasterLogger::log_set_sensitivity_pb(
    const SinglePbData &pb_data) {
  for (auto logger : _loggers) {
    logger->log_set_sensitivity_pb(pb_data);
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

void SensitivityMasterLogger::log_at_ending() {
  for (auto logger : _loggers) {
    logger->log_at_ending();
  }
}