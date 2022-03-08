#ifndef ANTARESXPANSION_SENSITIVITYILOGGER_H
#define ANTARESXPANSION_SENSITIVITYILOGGER_H

#include <string>

#include "SensitivityOutputData.h"

class SensitivityILogger {
 public:
  virtual ~SensitivityILogger() = default;

  virtual void display_message(const std::string &msg) = 0;
  virtual void log_at_start() = 0;
  virtual void log_set_sensitivity_pb(const SinglePbData &pb_data) = 0;
  virtual void log_begin_pb_resolution(const SinglePbData &pb_data) = 0;
  virtual void log_pb_solution(const SinglePbData &pb_data) = 0;
  virtual void log_at_ending() = 0;
};

#endif  // ANTARESXPANSION_SENSITIVITYILOGGER_H