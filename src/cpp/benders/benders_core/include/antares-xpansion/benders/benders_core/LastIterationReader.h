#ifndef __LASTITERATIONREADER_H__
#include <json/reader.h>

#include <filesystem>

#include "ILogger.h"

class LastIterationReader {
 public:
  explicit LastIterationReader(
      const std::filesystem::path& last_iteration_file);
  std::pair<LogData, LogData> LastIterationData();
  bool IsLastIterationFileValid() const;

 private:
  LogData GetIterationData(const std::string& iteration_name);
  std::filesystem::path last_iteration_file_;
  Json::Value last_iteration_file_content_;
};

#define __LASTITERATIONREADER_H__
#endif  // __LASTITERATIONREADER_H__