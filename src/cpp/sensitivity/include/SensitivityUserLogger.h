#ifndef ANTARESXPANSION_SENSITIVITYUSERLOGGER_H
#define ANTARESXPANSION_SENSITIVITYUSERLOGGER_H

#include <ostream>

#include "SensitivityILogger.h"

class SensitivityUserLogger : public SensitivityILogger {
 public:
  explicit SensitivityUserLogger(std::ostream &stream);
  ~SensitivityUserLogger() = default;

  void display_message(const std::string &msg) override;
  void log_at_start() override;
  void log_set_sensitivity_pb(const SinglePbData &pb_data) override;
  void log_begin_pb_resolution(const SinglePbData &pb_data) override;
  void log_pb_solution(const SinglePbData& pb_data) override;
  void log_at_ending() override;

  private:
  std::ostream &_stream;
};

#endif  // ANTARESXPANSION_SENSITIVITYUSERLOGGER_H