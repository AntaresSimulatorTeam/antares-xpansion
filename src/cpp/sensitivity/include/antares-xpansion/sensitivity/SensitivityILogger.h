#ifndef ANTARESXPANSION_SENSITIVITYILOGGER_H
#define ANTARESXPANSION_SENSITIVITYILOGGER_H

#include <string>

#include "antares-xpansion/sensitivity/SensitivityInputReader.h"
#include "antares-xpansion/sensitivity/SensitivityOutputData.h"

class SensitivityILogger {
 public:
  virtual ~SensitivityILogger() = default;

  virtual void display_message(const std::string &msg) = 0;
  virtual void log_at_start(const SensitivityInputData &input_data) = 0;
  virtual void log_begin_pb_resolution(const SinglePbData &pb_data) = 0;
  virtual void log_pb_solution(const SinglePbData &pb_data) = 0;
  virtual void log_summary(const SensitivityInputData &input_data,
                           const std::vector<SinglePbData> &pbs_data) = 0;
  virtual void log_at_ending() = 0;
};

#endif  // ANTARESXPANSION_SENSITIVITYILOGGER_H