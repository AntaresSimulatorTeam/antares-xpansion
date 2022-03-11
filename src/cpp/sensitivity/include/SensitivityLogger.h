#ifndef ANTARESXPANSION_SENSITIVITYLOGGER_H
#define ANTARESXPANSION_SENSITIVITYLOGGER_H

#include <ostream>

#include "SensitivityILogger.h"

const std::string indent_1("\t");
const std::string EUROS(" e");
const std::string MILLON_EUROS(" Me");
const std::string MW(" MW");

class SensitivityLogger : public SensitivityILogger {
 public:
  explicit SensitivityLogger(std::ostream &stream);

  void display_message(const std::string &msg) override;
  void log_at_start(const SensitivityOutputData &output_data) override;
  void log_begin_pb_resolution(const SinglePbData &pb_data) override;
  void log_pb_solution(const SinglePbData &pb_data) override;
  void log_summary(const SensitivityOutputData &output_data) override;
  void log_at_ending() override;

 private:
  std::ostream &_stream;

  void log_projection_summary(const std::vector<SinglePbData> &projection_data);
  void log_capex_summary(const std::vector<SinglePbData> &capex_data);
  std::map<std::string, std::map<std::string, double>> get_investment_intervals(
      const std::vector<SinglePbData> &projection_data) const;
  std::string get_objective_unit(const SinglePbData &pb_data) const;
  std::string format_objective(const SinglePbData &pb_data) const;
};

#endif  // ANTARESXPANSION_SENSITIVITYLOGGER_H