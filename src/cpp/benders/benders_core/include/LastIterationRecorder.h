#ifndef __LASTITERATIONRECORDER__H__
#define __LASTITERATIONRECORDER__H__
#include <json/writer.h>

#include <filesystem>

#include "core/ILogger.h"
class LastIterationRecorder {
 public:
  LastIterationRecorder(const std::filesystem::path &last_iteration_file)
      : _output_file(last_iteration_file) {}
  void save_last_iteration(const LogData &iteration_log_data);

 private:
  void _fill_output();
  std::filesystem::path _output_file;
  LogData _last_iteration_data;
  Json::Value _output;
};
#endif  //__LASTITERATIONRECORDER__H__