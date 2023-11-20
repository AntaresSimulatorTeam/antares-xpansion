
#pragma once
#include <filesystem>

#include "BendersMathLogger.h"
class MathLoggerFile : public MathLogger {
 public:
  explicit MathLoggerFile(const std::filesystem::path& log_file);
  using MathLogger::MathLogger;

 private:
  std::ofstream file_stream_;
};

class MathLoggerOstream : public MathLogger {
 public:
  explicit MathLoggerOstream() : MathLogger(&std::cout) {}
};
