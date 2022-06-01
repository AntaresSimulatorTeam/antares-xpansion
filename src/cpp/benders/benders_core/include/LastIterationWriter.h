#ifndef __LASTITERATIONWRITER__H__
#define __LASTITERATIONWRITER__H__
#include <json/writer.h>

#include <filesystem>

#include "core/ILogger.h"
class LastIterationWriter {
 public:
  explicit LastIterationWriter(const std::filesystem::path &last_iteration_file)
      : _output_file(last_iteration_file) {}
  void save_best_and_last_iterations(const LogData &best_iteration_log_data,
                                     const LogData &last_iteration_log_data);

 private:
  void _fill_output(const std::string &iteration_name,
                    const LogData &iteration_data);
  std::filesystem::path _output_file;
  LogData _last_iteration_data;
  LogData _best_iteration_data;
  Json::Value _output;
};
#endif  //__LASTITERATIONWRITER__H__