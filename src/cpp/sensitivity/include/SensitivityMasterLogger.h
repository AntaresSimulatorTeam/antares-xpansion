#include <list>
#include <memory>

#include "SensitivityILogger.h"

class SensitivityMasterLogger : public SensitivityILogger {
 public:
  SensitivityMasterLogger() = default;
  ~SensitivityMasterLogger() = default;

  void addLogger(const std::shared_ptr<SensitivityILogger> &logger) {
    _loggers.push_back(logger);
  };

  void display_message(const std::string &msg) override;
  void log_at_start(const SensitivityOutputData &output_data) override;
  void log_set_sensitivity_pb(const SinglePbData &pb_data) override;
  void log_begin_pb_resolution(const SinglePbData &pb_data) override;
  void log_pb_solution(const SinglePbData& pb_data) override;
  virtual void log_summary(const SensitivityOutputData &output_data) override;
  void log_at_ending() override;

 private:
  std::list<std::shared_ptr<SensitivityILogger>> _loggers;
};
