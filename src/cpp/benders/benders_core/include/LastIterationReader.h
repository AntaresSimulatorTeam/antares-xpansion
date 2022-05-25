#ifndef __LASTITERATIONREADER_H__
#include <json/reader.h>

#include <filesystem>

#include "core/ILogger.h"

class LastIterationReader {
 public:
  LastIterationReader(const std::filesystem::path& last_iteration_file);
  std::pair<LogData, LogData> last_iteration_data();
  bool is_last_iteration_file_valid() const;

 private:
  LogData _get_iteration_data(const std::string& iteration_name);
  std::filesystem::path _last_iteration_file;
  Json::Value _last_iteration_file_content;
};

#define __LASTITERATIONREADER_H__
#endif  // __LASTITERATIONREADER_H__