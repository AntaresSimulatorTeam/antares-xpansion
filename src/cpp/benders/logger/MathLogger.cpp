#include "logger/MathLogger.h"

#include <iomanip>
#include <sstream>

double getDurationNotSolving(double interation, double master,
                             double subproblems) {
  return interation - master - subproblems;
}

void MathLoggerBase::setHeadersList() {
  auto type = HeadersType();
  HeadersManager headers_manager(type, BENDERSMETHOD::BENDERS);
  MathLogger::setHeadersList(headers_manager.headers_list);
}

void MathLogger::setHeadersList(const std::vector<std::string>& headers) {
  headers_.clear();
  headers_ = headers;
}

void MathLoggerBase::Print(const CurrentIterationData& data) {
  auto type = HeadersType();

  LogsDestination() << data.it;
  LogsDestination() << std::scientific << std::setprecision(10) << data.lb;
  LogsDestination() << std::scientific << std::setprecision(10) << data.ub;
  LogsDestination() << std::scientific << std::setprecision(10) << data.best_ub;
  LogsDestination() << std::scientific << std::setprecision(2)
                    << data.best_ub - data.lb;
  LogsDestination() << std::scientific << std::setprecision(2)
                    << (data.best_ub - data.lb) / data.best_ub;
  LogsDestination() << data.min_simplexiter;
  LogsDestination() << data.max_simplexiter;
  if (type == HEADERSTYPE::LONG) {
    LogsDestination() << data.number_of_subproblem_solved;
    LogsDestination() << data.cumulative_number_of_subproblem_solved;
  }

  // LogsDestination()  << data.deletedcut;
  LogsDestination() << std::setprecision(2) << data.elapsed_time;
  LogsDestination() << std::setprecision(2) << data.timer_master;
  LogsDestination() << std::setprecision(2) << data.subproblems_walltime;
  if (type == HEADERSTYPE::LONG) {
    LogsDestination() << std::setprecision(2)
                      << data.subproblems_cumulative_cputime;
    LogsDestination() << std::setprecision(2)
                      << getDurationNotSolving(data.elapsed_time,
                                               data.timer_master,
                                               data.subproblems_walltime);
  }
  LogsDestination() << std::endl;
}

void MathLoggerBendersByBatch::setHeadersList() {
  auto type = HeadersType();
  HeadersManager headers_manager(type, BENDERSMETHOD::BENDERSBYBATCH);

  MathLogger::setHeadersList(headers_manager.headers_list);
}

void MathLoggerBendersByBatch::Print(const CurrentIterationData& data) {
  auto type = HeadersType();

  LogsDestination() << data.it;
  LogsDestination() << std::scientific << std::setprecision(10) << data.lb;
  LogsDestination() << data.min_simplexiter;
  LogsDestination() << data.max_simplexiter;
  LogsDestination() << data.number_of_subproblem_solved;

  if (type == HEADERSTYPE::LONG) {
    LogsDestination() << data.cumulative_number_of_subproblem_solved;
  }
  LogsDestination() << std::setprecision(2) << data.elapsed_time;
  LogsDestination() << std::setprecision(2) << data.timer_master;
  LogsDestination() << std::setprecision(2) << data.subproblems_walltime;
  if (type == HEADERSTYPE::LONG) {
    LogsDestination() << std::setprecision(2)
                      << data.subproblems_cumulative_cputime;
    LogsDestination() << std::setprecision(2)
                      << getDurationNotSolving(data.elapsed_time,
                                               data.timer_master,
                                               data.subproblems_walltime);
  }

  LogsDestination() << std::endl;
}

MathLoggerFile::MathLoggerFile(const BENDERSMETHOD& method,
                               const std::filesystem::path& filename,
                               std::streamsize width)
    : MathLoggerImplementation(method, filename, width, HEADERSTYPE::LONG) {}