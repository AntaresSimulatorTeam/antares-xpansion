#include "logger/MathLogger.h"

#include <iostream>

#include "LoggerUtils.h"
void MathLogger::write_header() {}

MathLoggerFile::MathLoggerFile(const std::filesystem::path &filename)
    : MathLogger(&file_stream_) {
  file_stream_.open(filename, std::ofstream::out | std::ofstream::app);
  if (file_stream_.fail()) {
    std::cerr << PrefixMessage(LogUtils::LOGLEVEL::ERR, data.CONTEXT)
              << "Invalid file name passed as parameter" << std::endl;
  }
}