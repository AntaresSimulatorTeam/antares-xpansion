
#pragma once
#include <filesystem>

#include "BendersMathLogger.h"
#include "LoggerUtils.h"

class MathLoggerFile : public MathLoggerImplementation {
 public:
  explicit MathLoggerFile(const BENDERSMETHOD& method,
                          const std::filesystem::path& log_file,
                          std::streamsize width = 40);

 private:
  std::ofstream file_stream_;
};
class MathLoggerOstream : public MathLoggerImplementation {
 public:
  explicit MathLoggerOstream(const BENDERSMETHOD& method,
                             std::streamsize width = 20)
      : MathLoggerImplementation(method, &std::cout, width,
                                 HEADERSTYPE::SHORT) {}
};
