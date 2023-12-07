#include "logger/MathLogger.h"

#include <iomanip>
#include <sstream>

double getDurationNotDoingMasterOrSubproblems(double benders, double master,
                                              double subproblems) {
  return benders - master - subproblems;
}

void MathLoggerBase::setHeadersList() {
  MathLogger::setHeadersList({ITERATION, LB, UB, BESTUB, ABSOLUTE_GAP,
                              RELATIVE_GAP, MINSIMPLEX, MAXSIMPLEX, TIMEMASTER,
                              SUB_PROBLEMS_TIME_CPU, SUB_PROBLEMS_TIME_WALL,
                              TIME_NOT_DOING_MASTER_OR_SUB_PROBLEMS_WALL});
}

void MathLogger::setHeadersList(const std::list<std::string>& headers) {
  headers_.clear();
  headers_ = headers;
}

void MathLoggerBase::Print(const CurrentIterationData& data) {
  LogsDestination() << std::setw(10) << data.it;
  if (data.lb == -1e20)
    LogsDestination() << std::setw(20) << "-INF";
  else
    LogsDestination() << std::setw(20) << std::scientific
                      << std::setprecision(10) << data.lb;
  if (data.ub == +1e20)
    LogsDestination() << std::setw(20) << "+INF";
  else
    LogsDestination() << std::setw(20) << std::scientific
                      << std::setprecision(10) << data.ub;
  if (data.best_ub == +1e20)
    LogsDestination() << std::setw(20) << "+INF";
  else
    LogsDestination() << std::setw(20) << std::scientific
                      << std::setprecision(10) << data.best_ub;
  LogsDestination() << std::setw(15) << std::scientific << std::setprecision(2)
                    << data.best_ub - data.lb;

  LogsDestination() << std::setw(15) << std::scientific << std::setprecision(2)
                    << (data.best_ub - data.lb) / data.best_ub;

  LogsDestination() << std::setw(15) << data.min_simplexiter;
  LogsDestination() << std::setw(15) << data.max_simplexiter;

  // LogsDestination() << std::setw(15) << data.deletedcut;
  LogsDestination() << std::setw(15) << std::setprecision(2)
                    << data.timer_master;
  LogsDestination() << std::setw(15) << std::setprecision(2)
                    << data.subproblems_cputime;
  LogsDestination() << std::setw(15) << std::setprecision(2)
                    << data.subproblems_walltime;
  LogsDestination() << std::setw(15) << std::setprecision(2)
                    << getDurationNotDoingMasterOrSubproblems(
                           data.elapsed_time, data.timer_master,
                           data.subproblems_walltime);

  LogsDestination() << std::endl;
}

void MathLoggerBendersByBatch::setHeadersList() {
  MathLogger::setHeadersList({ITERATION, LB, MINSIMPLEX, MAXSIMPLEX, TIMEMASTER,
                              SUB_PROBLEMS_TIME_CPU, SUB_PROBLEMS_TIME_WALL,
                              TIME_NOT_DOING_MASTER_OR_SUB_PROBLEMS_WALL});
}

void MathLoggerBendersByBatch::Print(const CurrentIterationData& data) {
  LogsDestination() << std::setw(10) << data.it;
  if (data.lb == -1e20)
    LogsDestination() << std::setw(20) << "-INF";
  else
    LogsDestination() << std::setw(20) << std::scientific
                      << std::setprecision(10) << data.lb;

  LogsDestination() << std::setw(15) << data.min_simplexiter;
  LogsDestination() << std::setw(15) << data.max_simplexiter;

  // LogsDestination() << std::setw(15) << data.deletedcut;
  LogsDestination() << std::setw(15) << std::setprecision(2)
                    << data.timer_master;
  LogsDestination() << std::setw(15) << std::setprecision(2)
                    << data.subproblems_cumulative_cputime;
  LogsDestination() << std::setw(15) << std::setprecision(2)
                    << data.subproblems_walltime;
  LogsDestination() << std::setw(15) << std::setprecision(2)
                    << getDurationNotDoingMasterOrSubproblems(
                           data.elapsed_time, data.timer_master,
                           data.subproblems_walltime);

  LogsDestination() << std::endl;
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