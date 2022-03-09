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
  void log_begin_pb_resolution(const SinglePbData &pb_data) override;
  void log_pb_solution(const SinglePbData &pb_data) override;
  virtual void log_summary(const SensitivityOutputData &output_data) override;
  void log_at_ending() override;

 private:
  std::ostream &_stream;

  void log_projection_summary(const std::vector<SinglePbData> &projection_data);
  void log_capex_summary(const std::vector<SinglePbData> &capex_data);
  std::map<std::string, std::map<std::string, double>> get_investment_intervals(
      const std::vector<SinglePbData> &projection_data);
  std::string get_objective_unit(const SinglePbData &pb_data);
  std::string format_objective(const SinglePbData &pb_data);
};

#endif  // ANTARESXPANSION_SENSITIVITYUSERLOGGER_H