#include "logger/MathLogger.h"

#include <iomanip>
#include <sstream>

#include "LoggerUtils.h"
std::string Indent(int size) { return std::string(size, ' '); }

void MathLogger::write_header() {
  log_destination << Indent(10) << "ITE";
  log_destination << Indent(20) << "LB";
  log_destination << Indent(20) << "UB";
  log_destination << Indent(20) << "BESTUB";
  log_destination << Indent(15) << "ABSOLUTE GAP";
  log_destination << Indent(15) << "RELATIVE GAP";

  log_destination << Indent(15) << "MINSIMPLEX";
  log_destination << Indent(15) << "MAXSIMPLEX";

  log_destination << Indent(15) << "TIMEMASTER";
  log_destination << Indent(15) << "TIMESLAVES";

  log_destination << std::endl;
}

void MathLogger::Print(const CurrentIterationData& data) {
  log_destination << Indent(10) << data.it;
  if (data.lb == -1e20)
    log_destination << Indent(20) << "-INF";
  else
    log_destination << Indent(20) << std::scientific << std::setprecision(10)
                    << data.lb;
  if (data.ub == +1e20)
    log_destination << Indent(20) << "+INF";
  else
    log_destination << Indent(20) << std::scientific << std::setprecision(10)
                    << data.ub;
  if (data.best_ub == +1e20)
    log_destination << Indent(20) << "+INF";
  else
    log_destination << Indent(20) << std::scientific << std::setprecision(10)
                    << data.best_ub;
  log_destination << Indent(15) << std::scientific << std::setprecision(2)
                  << data.best_ub - data.lb;

  log_destination << Indent(15) << std::scientific << std::setprecision(2)
                  << (data.best_ub - data.lb) /
                         data.best_ub;

  log_destination << Indent(15) << data.min_simplexiter;
  log_destination << Indent(15) << data.max_simplexiter;

  // log_destination << Indent(15) << data.deletedcut;
  log_destination << Indent(15) << std::setprecision(2) << data.timer_master;
  log_destination << Indent(15) << std::setprecision(2)
                  << data.subproblems_walltime;

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
