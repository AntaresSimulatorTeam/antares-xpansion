#include "logger/MathLogger.h"

#include <iomanip>
#include <iostream>

#include "LoggerUtils.h"

void MathLogger::write_header() {
  log_destination << std::setw(10) << "ITE";
  log_destination << std::setw(20) << "LB";
  log_destination << std::setw(20) << "LEV";
  log_destination << std::setw(20) << "UB";
  log_destination << std::setw(20) << "BESTUB";
  log_destination << std::setw(15) << "GAP";

  log_destination << std::setw(15) << "MINSIMPLEX";
  log_destination << std::setw(15) << "MAXSIMPLEX";

  log_destination << std::setw(15) << "DELETEDCUT";
  log_destination << std::setw(15) << "TIMEMASTER";
  log_destination << std::setw(15) << "TIMESLAVES";

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