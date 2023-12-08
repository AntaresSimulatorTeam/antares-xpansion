#include "logger/MathLogger.h"

#include <iomanip>
#include <sstream>

double getDurationNotDoingMasterOrSubproblems(double benders, double master,
                                              double subproblems) {
  return benders - master - subproblems;
}

void MathLoggerBase::setHeadersList() {
  HeadersManager headers(HeadersType());
  MathLogger::setHeadersList(
      {headers.ITERATION, headers.LB, headers.UB, headers.BESTUB,
       headers.ABSOLUTE_GAP, headers.RELATIVE_GAP, headers.MINSIMPLEX,
       headers.MAXSIMPLEX, headers.NUMBER_OF_SUBPROBLEM_SOLVED,
       headers.CUMULATIVE_NUMBER_OF_SUBPROBLEM_SOLVED, headers.BENDERS_TIME,
       headers.TIMEMASTER, headers.SUB_PROBLEMS_TIME_CPU,
       headers.SUB_PROBLEMS_TIME_WALL,
       headers.TIME_NOT_DOING_MASTER_OR_SUB_PROBLEMS_WALL});
}

void MathLogger::setHeadersList(const std::list<std::string>& headers) {
  headers_.clear();
  headers_ = headers;
}

void MathLoggerBase::Print(const CurrentIterationData& data) {
  LogsDestination() << data.it;
  if (data.lb == -1e20)
    LogsDestination() << "-INF";
  else
    LogsDestination() << std::scientific << std::setprecision(10) << data.lb;
  if (data.ub == +1e20)
    LogsDestination() << "+INF";
  else
    LogsDestination() << std::scientific << std::setprecision(10) << data.ub;
  if (data.best_ub == +1e20)
    LogsDestination() << "+INF";
  else
    LogsDestination() << std::scientific << std::setprecision(10)
                      << data.best_ub;
  LogsDestination() << std::scientific << std::setprecision(2)
                    << data.best_ub - data.lb;

  LogsDestination() << std::scientific << std::setprecision(2)
                    << (data.best_ub - data.lb) / data.best_ub;

  LogsDestination() << data.min_simplexiter;
  LogsDestination() << data.max_simplexiter;
  LogsDestination() << data.number_of_subproblem_solved;
  LogsDestination() << data.cumulative_number_of_subproblem_solved;

  // LogsDestination()  << data.deletedcut;
  LogsDestination() << std::setprecision(2) << data.elapsed_time;
  LogsDestination() << std::setprecision(2) << data.timer_master;
  LogsDestination() << std::setprecision(2) << data.subproblems_cputime;
  LogsDestination() << std::setprecision(2) << data.subproblems_walltime;
  LogsDestination() << std::setprecision(2)
                    << getDurationNotDoingMasterOrSubproblems(
                           data.elapsed_time, data.timer_master,
                           data.subproblems_walltime);

  LogsDestination() << std::endl;
}

void MathLoggerBendersByBatch::setHeadersList() {
  HeadersManager headers(HeadersType());
  MathLogger::setHeadersList(
      {headers.ITERATION, headers.LB, headers.MINSIMPLEX, headers.MAXSIMPLEX,
       headers.NUMBER_OF_SUBPROBLEM_SOLVED,
       headers.CUMULATIVE_NUMBER_OF_SUBPROBLEM_SOLVED, headers.BENDERS_TIME,
       headers.TIMEMASTER, headers.SUB_PROBLEMS_TIME_CPU,
       headers.SUB_PROBLEMS_TIME_WALL,
       headers.TIME_NOT_DOING_MASTER_OR_SUB_PROBLEMS_WALL});
}

void MathLoggerBendersByBatch::Print(const CurrentIterationData& data) {
  LogsDestination() << data.it;
  if (data.lb == -1e20)
    LogsDestination() << "-INF";
  else
    LogsDestination() << std::scientific << std::setprecision(10) << data.lb;

  LogsDestination() << data.min_simplexiter;
  LogsDestination() << data.max_simplexiter;

  // LogsDestination()  << data.deletedcut;
  LogsDestination() << data.number_of_subproblem_solved;
  LogsDestination() << data.cumulative_number_of_subproblem_solved;
  LogsDestination() << std::setprecision(2) << data.elapsed_time;
  LogsDestination() << std::setprecision(2) << data.timer_master;
  LogsDestination() << std::setprecision(2)
                    << data.subproblems_cumulative_cputime;
  LogsDestination() << std::setprecision(2) << data.subproblems_walltime;
  LogsDestination() << std::setprecision(2)
                    << getDurationNotDoingMasterOrSubproblems(
                           data.elapsed_time, data.timer_master,
                           data.subproblems_walltime);

  LogsDestination() << std::endl;
}

MathLoggerFile::MathLoggerFile(const BENDERSMETHOD& method,
                               const std::filesystem::path& filename,
                               std::streamsize width)
    : MathLoggerImplementation(method, &file_stream_, width,
                               HEADERSTYPE::LONG) {
  // TODO restart case?????????????
  file_stream_.open(filename, std::ofstream::out);
  if (file_stream_.fail()) {
    std::cerr << PrefixMessage(LogUtils::LOGLEVEL::ERR, MATHLOGGERCONTEXT)
              << "Invalid file name passed as parameter" << std::endl;
  }
}