
#pragma once
#include <filesystem>

#include "BendersMathLogger.h"
#include "LoggerUtils.h"

class MathLoggerFile : public MathLoggerImplementation {
 public:
  explicit MathLoggerFile(const BENDERSMETHOD& method,
                          const std::filesystem::path& log_file);

 private:
  std::ofstream file_stream_;
};
class MathLoggerOstream : public MathLoggerImplementation {
 public:
  explicit MathLoggerOstream(const BENDERSMETHOD& method)
      : MathLoggerImplementation(method, &std::cout) {}
};
