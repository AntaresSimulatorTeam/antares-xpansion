#ifndef ANTARESXPANSION_SENSITIVITYFILELOGGER_H
#define ANTARESXPANSION_SENSITIVITYFILELOGGER_H

#include <filesystem>
#include <fstream>
#include <memory>

#include "antares-xpansion/sensitivity/SensitivityILogger.h"
#include "antares-xpansion/sensitivity/SensitivityLogger.h"

class SensitivityFileLogger : public SensitivityILogger {
 public:
  explicit SensitivityFileLogger(const std::filesystem::path &filename);

  void display_message(const std::string &msg) override;
  void log_at_start(const SensitivityInputData &input_data) override;
  void log_begin_pb_resolution(const SinglePbData &pb_data) override;
  void log_pb_solution(const SinglePbData &pb_data) override;
  void log_summary(const SensitivityInputData &input_data,
                   const std::vector<SinglePbData> &pbs_data) override;
  void log_at_ending() override;

 private:
  std::ofstream _file;
  std::unique_ptr<SensitivityLogger> _userLog;
};

#endif  // ANTARESXPANSION_SENSITIVITYFILELOGGER_H