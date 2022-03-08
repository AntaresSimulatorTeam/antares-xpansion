#ifndef ANTARESXPANSION_SENSITIVITYUSERLOGGER_H
#define ANTARESXPANSION_SENSITIVITYUSERLOGGER_H

#include <ostream>

#include "SensitivityILogger.h"

class SensitivityUserLogger : public SensitivityILogger {
 public:
  explicit SensitivityUserLogger(std::ostream &stream);
  ~SensitivityUserLogger() = default;

  void display_message(const std::string &msg) override;
  void log_at_start(const SensitivityOutputData &output_data) override;
  void log_set_sensitivity_pb(const SinglePbData &pb_data) override;
  void log_begin_pb_resolution(const SinglePbData &pb_data) override;
  void log_pb_solution(const SinglePbData &pb_data) override;
  virtual void log_summary(const SensitivityOutputData &output_data) override;
  void log_at_ending() override;

 private:
  std::ostream &_stream;

  void log_projection_summary(const SensitivityOutputData &output_data);
  void log_capex_summary(const SensitivityOutputData &output_data);
  static bool is_capex_min(const SinglePbData &pb_data);
  static bool is_capex_max(const SinglePbData &pb_data);
};

#endif  // ANTARESXPANSION_SENSITIVITYUSERLOGGER_H