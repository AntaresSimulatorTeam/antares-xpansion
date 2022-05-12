#ifndef __LASTITERATIONREADER_H__
#include <json/reader.h>

#include <filesystem>

#include "core/ILogger.h"

class LastIterationReader {
 public:
  LastIterationReader(const std::filesystem::path& last_iteration_file);
  LogData last_iteration_data() const;

 private:
  std::filesystem::path _last_iteration_file;
  Json::Value _get_last_iteration_file_content() const;
};

#define __LASTITERATIONREADER_H__
#endif  // __LASTITERATIONREADER_H__