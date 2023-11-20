#include "logger/MathLogger.h"

#include <iomanip>
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

void MathLogger::Print(const MathLoggerData& data) {
  log_destination << Indent(10) << data.iteration;
  if (data.lower_bound == -1e20)
    log_destination << Indent(20) << "-INF";
  else
    log_destination << Indent(20) << std::scientific << std::setprecision(10)
                    << data.lower_bound;
  if (data.upper_bound == +1e20)
    log_destination << Indent(20) << "+INF";
  else
    log_destination << Indent(20) << std::scientific << std::setprecision(10)
                    << data.upper_bound;
  if (data.best_upper_bound == +1e20)
    log_destination << Indent(20) << "+INF";
  else
    log_destination << Indent(20) << std::scientific << std::setprecision(10)
                    << data.best_upper_bound;
  log_destination << Indent(15) << std::scientific << std::setprecision(2)
                  << data.best_upper_bound - data.lower_bound;

  log_destination << Indent(15) << std::scientific << std::setprecision(2)
                  << (data.best_upper_bound - data.lower_bound) /
                         data.best_upper_bound;

  log_destination << Indent(15) << data.min_simplexiter;
  log_destination << Indent(15) << data.max_simplexiter;

  log_destination << Indent(15) << data.deletedcut;
  log_destination << Indent(15) << std::setprecision(2) << data.time_master;
  log_destination << Indent(15) << std::setprecision(2)
                  << data.time_subproblems;

  log_destination << std::endl;
}

MathLoggerFile::MathLoggerFile(const std::filesystem::path &filename)
    : MathLogger(&file_stream_) {
  file_stream_.open(filename, std::ofstream::out | std::ofstream::app);
  if (file_stream_.fail()) {
    std::cerr << PrefixMessage(LogUtils::LOGLEVEL::ERR, MATHLOGGERCONTEXT)
              << "Invalid file name passed as parameter" << std::endl;
  }
}
