#ifndef ANTARESXPANSION_SENSITIVITYMASTERLOGGER_H
#define ANTARESXPANSION_SENSITIVITYMASTERLOGGER_H

#include <list>
#include <memory>

#include "antares-xpansion/sensitivity/SensitivityILogger.h"

class SensitivityMasterLogger : public SensitivityILogger {
 public:
  SensitivityMasterLogger() = default;

  void addLogger(const std::shared_ptr<SensitivityILogger> &logger) {
    _loggers.push_back(logger);
  };

  void display_message(const std::string &msg) override;
  void log_at_start(const SensitivityInputData &input_data) override;
  void log_begin_pb_resolution(const SinglePbData &pb_data) override;
  void log_pb_solution(const SinglePbData &pb_data) override;
  void log_summary(const SensitivityInputData &input_data,
                   const std::vector<SinglePbData> &pbs_data) override;
  void log_at_ending() override;

 private:
  std::list<std::shared_ptr<SensitivityILogger>> _loggers;
};

#endif  // ANTARESXPANSION_SENSITIVITYMASTERLOGGER_H