#include "logger/MathLogger.h"

#include <iomanip>
#include <sstream>

void MathLoggerBase::setHeadersList() {
  headers.clear();
  headers = {ITE,
             LB,
             UB,
             BESTUB,
             ABSOLUTE_GAP,
             RELATIVE_GAP,
             MINSIMPLEX,
             MAXSIMPLEX,
             TIMEMASTER,
             SUB_PROBLEMS_TIME_CPU,
             SUB_PROBLEMS_TIME_WALL};
}

void MathLogger::write_header() {
  setHeadersList();
  for (const auto& header : headers) {
    log_destination << header;
  }
  log_destination << std::endl;
}
void MathLoggerBase::Print(const CurrentIterationData& data) {
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
                  << (data.best_ub - data.lb) / data.best_ub;

  log_destination << Indent(15) << data.min_simplexiter;
  log_destination << Indent(15) << data.max_simplexiter;

  // log_destination << Indent(15) << data.deletedcut;
  log_destination << Indent(15) << std::setprecision(2) << data.timer_master;
  log_destination << Indent(15) << std::setprecision(2)
                  << data.subproblems_cputime;
  log_destination << Indent(15) << std::setprecision(2)
                  << data.subproblems_walltime;

  log_destination << std::endl;
}

void MathLoggerBendersByBatch::setHeadersList() {
  headers.clear();
  headers = {ITE,
             LB,
             MINSIMPLEX,
             MAXSIMPLEX,
             TIMEMASTER,
             SUB_PROBLEMS_TIME_CPU,
             SUB_PROBLEMS_TIME_WALL,
             TIME_NOT_DOING_MASTER_OR_SUB_PROBLEMS_WALL};
}

void MathLoggerBendersByBatch::Print(const CurrentIterationData& data) {
  log_destination << Indent(10) << data.it;
  if (data.lb == -1e20)
    log_destination << Indent(20) << "-INF";
  else
    log_destination << Indent(20) << std::scientific << std::setprecision(10)
                    << data.lb;

  log_destination << Indent(15) << data.min_simplexiter;
  log_destination << Indent(15) << data.max_simplexiter;

  // log_destination << Indent(15) << data.deletedcut;
  log_destination << Indent(15) << std::setprecision(2) << data.timer_master;
  log_destination << Indent(15) << std::setprecision(2)
                  << data.subproblems_cumulative_cputime;
  log_destination << Indent(15) << std::setprecision(2)
                  << data.subproblems_walltime;

  log_destination << std::endl;
}

MathLoggerFile::MathLoggerFile(const BENDERSMETHOD& method,
                               const std::filesystem::path& filename)
    : MathLoggerImplementation(method, &file_stream_) {
  // TODO restart case?????????????
  file_stream_.open(filename, std::ofstream::out);
  if (file_stream_.fail()) {
    std::cerr << PrefixMessage(LogUtils::LOGLEVEL::ERR, MATHLOGGERCONTEXT)
              << "Invalid file name passed as parameter" << std::endl;
  }
}