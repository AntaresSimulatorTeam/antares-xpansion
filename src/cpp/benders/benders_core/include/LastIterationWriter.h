#ifndef __LASTITERATIONWRITER__H__
#define __LASTITERATIONWRITER__H__
#include <json/writer.h>

#include <filesystem>

#include "ILogger.h"
class LastIterationWriter {
 public:
  explicit LastIterationWriter(const std::filesystem::path &last_iteration_file)
      : output_file_(last_iteration_file) {}
  void SaveBestAndLastIterations(const LogData &best_iteration_log_data,
                                 const LogData &last_iteration_log_data);

 private:
  void FillOutput(const std::string &iteration_name,
                  const LogData &iteration_data);
  std::filesystem::path output_file_;
  LogData last_iteration_data_;
  LogData best_iteration_data_;
  Json::Value output_;
};
#endif  //__LASTITERATIONWRITER__H__