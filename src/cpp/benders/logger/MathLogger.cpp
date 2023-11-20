#include "logger/MathLogger.h"

#include <iomanip>
#include <iostream>
#include <sstream>

#include "LoggerUtils.h"
std::string Indent(int size) {
  std::stringstream ss;
  ss << std::setw(size);
  return ss.str();
}

void MathLogger::write_header() {
  log_destination << Indent(10) << "ITE";
  log_destination << Indent(20) << "LB";
  log_destination << Indent(20) << "LEV";
  log_destination << Indent(20) << "UB";
  log_destination << Indent(20) << "BESTUB";
  log_destination << Indent(15) << "GAP";

  log_destination << Indent(15) << "MINSIMPLEX";
  log_destination << Indent(15) << "MAXSIMPLEX";

  log_destination << Indent(15) << "DELETEDCUT";
  log_destination << Indent(15) << "TIMEMASTER";
  log_destination << Indent(15) << "TIMESLAVES";

  log_destination << std::endl;
}

MathLoggerFile::MathLoggerFile(const std::filesystem::path &filename)
    : MathLogger(&file_stream_) {
  file_stream_.open(filename, std::ofstream::out | std::ofstream::app);
  if (file_stream_.fail()) {
    std::cerr << PrefixMessage(LogUtils::LOGLEVEL::ERR, data.CONTEXT)
              << "Invalid file name passed as parameter" << std::endl;
  }
}