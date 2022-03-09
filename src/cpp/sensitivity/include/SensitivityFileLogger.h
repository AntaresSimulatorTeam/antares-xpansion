#ifndef ANTARESXPANSION_SENSITIVITYFILELOGGER_H
#define ANTARESXPANSION_SENSITIVITYFILELOGGER_H

#include <fstream>
#include <memory>

#include "SensitivityILogger.h"
#include "SensitivityUserLogger.h"

class SensitivityFileLogger : public SensitivityILogger {
 public:
  explicit SensitivityFileLogger(const std::string &filename);
  ~SensitivityFileLogger();

  void display_message(const std::string &msg) override;
  void log_at_start(const SensitivityOutputData &output_data) override;
  void log_begin_pb_resolution(const SinglePbData &pb_data) override;
  void log_pb_solution(const SinglePbData& pb_data) override;
  virtual void log_summary(const SensitivityOutputData &output_data) override;
  void log_at_ending() override;

 private:
  std::ofstream _file;
  std::unique_ptr<SensitivityUserLogger> _userLog;
};

#endif  // ANTARESXPANSION_SENSITIVITYFILELOGGER_H