#ifndef ANTARESXPANSION_SENSITIVITYLOGGER_H
#define ANTARESXPANSION_SENSITIVITYLOGGER_H

#include <ostream>

#include "antares-xpansion/sensitivity/SensitivityILogger.h"

const std::string indent_1("\t");
const std::string EUROS(" e");
const std::string MILLON_EUROS(" Me");
const std::string MW(" MW");

class SensitivityLogger : public SensitivityILogger {
 public:
  explicit SensitivityLogger(std::ostream &stream);

  void display_message(const std::string &msg) override;
  void log_at_start(const SensitivityInputData &input_data) override;
  void log_begin_pb_resolution(const SinglePbData &pb_data) override;
  void log_pb_solution(const SinglePbData &pb_data) override;
  void log_summary(const SensitivityInputData &input_data,
                   const std::vector<SinglePbData> &pbs_data) override;
  void log_at_ending() override;

 private:
  std::ostream &_stream;

  void log_benders_overall_cost(const double &best_ub);
  void log_benders_capex(const double &best_capex);
  void log_benders_solution(
      const std::map<std::string, double> &benders_solution);
  void log_epsilon(const double &epsilon);

  void log_projection_summary(
      const std::vector<SinglePbData> &projection_data,
      const std::map<std::string, std::pair<double, double>>
          &candidates_bounds);
  void log_capex_summary(const std::vector<SinglePbData> &capex_data);
};

#endif  // ANTARESXPANSION_SENSITIVITYLOGGER_H