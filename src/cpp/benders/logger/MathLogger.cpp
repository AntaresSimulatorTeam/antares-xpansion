#include "antares-xpansion/benders/logger/MathLogger.h"

MathLoggerFile::MathLoggerFile(const BENDERSMETHOD& method,
                               const std::filesystem::path& filename,
                               std::streamsize width)
    : MathLoggerImplementation(method, filename, width, HEADERSTYPE::LONG) {}

void MathLoggerFile::display_message(const std::string& msg) {
  // keep empty
}

void MathLoggerFile::display_message(const std::string& msg,
                                     LogUtils::LOGLEVEL level,
                                     const std::string& context) {
  // keep empty
}

void MathLoggerOstream::PrintIterationSeparatorBegin() {
  // keep empty
}

void MathLoggerOstream::PrintIterationSeparatorEnd() {
  // keep empty
}
void MathLoggerFile::PrintIterationSeparatorBegin() {
  // keep empty
}

void MathLoggerFile::PrintIterationSeparatorEnd() {
  // keep empty
}
