#include "antares-xpansion/benders/benders_core/BendersMathLogger.h"

#include <iomanip>
#include <sstream>

#include "antares-xpansion/xpansion_interfaces/LogUtils.h"
#include "antares-xpansion/xpansion_interfaces/LoggerUtils.h"

HeadersManager::HeadersManager(HEADERSTYPE type, const BENDERSMETHOD& method)
    : type_(type), method_(method) {}

std::vector<std::string> HeadersManager::HeadersList() {
  std::vector<std::string> headers_list;

  headers_list.push_back("Ite");
  headers_list.push_back("Lb");
  if (method_ == BENDERSMETHOD::BENDERS) {
    headers_list.push_back("Ub");
    headers_list.push_back("BestUb");
    headers_list.push_back("AbsGap");
    headers_list.push_back("RelGap");
  }
  headers_list.push_back("MinSpx");
  headers_list.push_back("MaxSpx");

  if (type_ == HEADERSTYPE::LONG ||
      method_ == BENDERSMETHOD::BENDERS_BY_BATCH) {
    headers_list.push_back("NbSubPbSolv");
  }

  if (type_ == HEADERSTYPE::LONG) {
    headers_list.push_back("CumulNbSubPbSolv");
  }
  headers_list.push_back("IteTime (s)");
  headers_list.push_back("MasterTime (s)");
  headers_list.push_back("SPWallTime (s)");

  if (type_ == HEADERSTYPE::LONG) {
    headers_list.push_back("SPCpuTime (s)");
    headers_list.push_back("NotSolvingWallTime (s)");
  }

  return headers_list;
}

HeadersManagerExternalLoop::HeadersManagerExternalLoop(
    HEADERSTYPE type, const BENDERSMETHOD& method)
    : HeadersManager(type, method) {}

std::vector<std::string> HeadersManagerExternalLoop::HeadersList() {
  std::vector<std::string> headers_list;
  headers_list.push_back("Outer loop");
  headers_list.push_back("Max Criterion");
  headers_list.push_back("Area Max Criterion");
  headers_list.push_back("Bilevel best ub");
  headers_list.push_back("Lambda");
  headers_list.push_back("Lambda min");
  headers_list.push_back("Lambda max");
  auto base_headers = HeadersManager::HeadersList();
  std::move(base_headers.begin(), base_headers.end(),
            std::back_inserter(headers_list));
  return headers_list;
}

LogDestination::LogDestination(std::streamsize width)
    : stream_(&std::cout), width_(width) {
  (*stream_) << std::unitbuf;
}

LogDestination::LogDestination(const std::filesystem::path& file_path,
                               std::streamsize width)
    : width_(width) {
  file_stream_.open(file_path, std::ofstream::out | std::ofstream::trunc);
  if (file_stream_.is_open()) {
    stream_ = &file_stream_;
    (*stream_) << std::unitbuf;
  } else {
    std::ostringstream err_msg;
    err_msg << PrefixMessage(LogUtils::LOGLEVEL::WARNING, MATHLOGGERCONTEXT)
            << "Could not open the file: "
            << std::quoted(file_path.string().c_str()) << "\n";
    std::cerr << err_msg.str();
  }
}
void LogDestination::setDelimiter(const std::string& delimiter) {
  delimiter_ = delimiter;
}

void MathLoggerBehaviour::write_header() {
  setHeadersList();
  LogsDestination().InsertDelimiter();
  for (const auto& header : Headers()) {
    LogsDestination() << header;
    LogsDestination().InsertDelimiter();
  }
  LogsDestination() << std::endl;
}

void MathLoggerBehaviour::PrintIterationSeparatorBegin() {
  std::string sep_msg("/*\\");
  sep_msg += std::string(74, '-');
  LogsDestination() << sep_msg << std::endl;
}

void MathLoggerBehaviour::PrintIterationSeparatorEnd() {
  std::string sep_msg(74, '-');
  sep_msg = "\\*/" + sep_msg;
  LogsDestination() << sep_msg << std::endl;
}

MathLogger::MathLogger(const std::filesystem::path& file_path,
                       std::streamsize width, HEADERSTYPE type)
    : log_destination_(file_path, width), type_(type) {}

MathLogger::MathLogger(std::streamsize width, HEADERSTYPE type)
    : log_destination_(width), type_(type) {}

void MathLogger::display_message(const std::string& str) {
  LogsDestination() << str << std::endl;
}
void MathLogger::display_message(const std::string& str,
                                 LogUtils::LOGLEVEL level,
                                 const std::string& context) {
  LogsDestination() << PrefixMessage(level, context) << str << std::endl;
}
std::vector<std::string> MathLogger::Headers() const { return headers_; }

LogDestination& MathLogger::LogsDestination() { return log_destination_; }

HEADERSTYPE MathLogger::HeadersType() const { return type_; }

void MathLoggerBase::Print(const CurrentIterationData& data) {
  PrintBendersData(LogsDestination(), data, HeadersType(),
                   BENDERSMETHOD::BENDERS);
}

void MathLoggerBase::setHeadersList() {
  auto type = HeadersType();
  HeadersManager headers_manager(type, BENDERSMETHOD::BENDERS);
  MathLogger::setHeadersList(headers_manager.HeadersList());
}

void MathLoggerBaseExternalLoop::setHeadersList() {
  auto type = HeadersType();
  HeadersManagerExternalLoop headers_manager(type, BENDERSMETHOD::BENDERS);
  MathLogger::setHeadersList(headers_manager.HeadersList());
}

void MathLogger::setHeadersList(const std::vector<std::string>& headers) {
  headers_.clear();
  headers_ = headers;
}

double getDurationNotSolving(double iteration, double master,
                             double subproblems) {
  return iteration - master - subproblems;
}

void PrintBendersData(LogDestination& log_destination,
                      const CurrentIterationData& data, const HEADERSTYPE& type,
                      const BENDERSMETHOD& method) {
  log_destination.InsertDelimiter();
  log_destination << data.it;
  log_destination.InsertDelimiter();
  log_destination << std::scientific << std::setprecision(10) << data.lb;
  log_destination.InsertDelimiter();
  if (method == BENDERSMETHOD::BENDERS) {
    log_destination << std::scientific << std::setprecision(10) << data.ub;
    log_destination.InsertDelimiter();
    log_destination << std::scientific << std::setprecision(10) << data.best_ub;
    log_destination.InsertDelimiter();
    log_destination << std::scientific << std::setprecision(2)
                    << data.best_ub - data.lb;
    log_destination.InsertDelimiter();
    log_destination << std::scientific << std::setprecision(2)
                    << (data.best_ub - data.lb) / data.best_ub;
    log_destination.InsertDelimiter();
  }
  log_destination << data.min_simplexiter;
  log_destination.InsertDelimiter();
  log_destination << data.max_simplexiter;
  log_destination.InsertDelimiter();
  if (type == HEADERSTYPE::LONG || method == BENDERSMETHOD::BENDERS_BY_BATCH) {
    log_destination << data.number_of_subproblem_solved;
    log_destination.InsertDelimiter();
  }
  if (type == HEADERSTYPE::LONG) {
    log_destination << data.cumulative_number_of_subproblem_solved;
    log_destination.InsertDelimiter();
  }

  log_destination << std::setprecision(2) << data.iteration_time;
  log_destination.InsertDelimiter();

  log_destination << std::setprecision(2) << data.timer_master;
  log_destination.InsertDelimiter();

  log_destination << std::setprecision(2) << data.subproblems_walltime;
  log_destination.InsertDelimiter();

  if (type == HEADERSTYPE::LONG) {
    log_destination << std::setprecision(2)
                    << data.subproblems_cumulative_cputime;
    log_destination.InsertDelimiter();
    log_destination << std::setprecision(2)
                    << getDurationNotSolving(data.iteration_time,
                                             data.timer_master,
                                             data.subproblems_walltime);
    log_destination.InsertDelimiter();
  }
  log_destination << std::endl;
}

void PrintExternalLoopData(LogDestination& log_destination,
                           const CurrentIterationData& data,
                           const HEADERSTYPE& type,
                           const BENDERSMETHOD& method) {
  log_destination.InsertDelimiter();
  log_destination << data.outer_loop_current_iteration_data.benders_num_run;
  log_destination.InsertDelimiter();
  log_destination << std::scientific << std::setprecision(10)
                  << data.outer_loop_current_iteration_data.max_criterion;
  log_destination.InsertDelimiter();
  log_destination << data.outer_loop_current_iteration_data.max_criterion_area;
  log_destination.InsertDelimiter();

  log_destination
      << std::scientific << std::setprecision(10)
      << data.outer_loop_current_iteration_data.outer_loop_bilevel_best_ub;
  log_destination.InsertDelimiter();
  log_destination
      << std::scientific << std::setprecision(10)
      << data.outer_loop_current_iteration_data.external_loop_lambda;
  log_destination.InsertDelimiter();
  log_destination
      << std::scientific << std::setprecision(10)
      << data.outer_loop_current_iteration_data.external_loop_lambda_min;
  log_destination.InsertDelimiter();
  log_destination
      << std::scientific << std::setprecision(10)
      << data.outer_loop_current_iteration_data.external_loop_lambda_max;
  PrintBendersData(log_destination, data, type, method);
}
void MathLoggerBaseExternalLoop::Print(const CurrentIterationData& data) {
  PrintExternalLoopData(LogsDestination(), data, HeadersType(),
                        BENDERSMETHOD::BENDERS);
}

void MathLoggerBendersByBatch::setHeadersList() {
  auto type = HeadersType();
  HeadersManager headers_manager(type, BENDERSMETHOD::BENDERS_BY_BATCH);

  MathLogger::setHeadersList(headers_manager.HeadersList());
}

void MathLoggerBendersByBatchExternalLoop::setHeadersList() {
  auto type = HeadersType();
  HeadersManagerExternalLoop headers_manager(type,
                                             BENDERSMETHOD::BENDERS_BY_BATCH);
  MathLogger::setHeadersList(headers_manager.HeadersList());
}

void MathLoggerBendersByBatch::Print(const CurrentIterationData& data) {
  PrintBendersData(LogsDestination(), data, HeadersType(),
                   BENDERSMETHOD::BENDERS_BY_BATCH);
}
void MathLoggerBendersByBatchExternalLoop::Print(
    const CurrentIterationData& data) {
  PrintExternalLoopData(LogsDestination(), data, HeadersType(),
                        BENDERSMETHOD::BENDERS);
}
void MathLoggerDriver::add_logger(
    std::shared_ptr<MathLoggerImplementation> logger) {
  if (logger) {
    math_loggers_.push_back(logger);
  }
}

void MathLoggerDriver::Print(const CurrentIterationData& data) {
  for (auto logger : math_loggers_) {
    logger->Print(data);
  }
}

void MathLoggerDriver::write_header() {
  for (auto logger : math_loggers_) {
    logger->write_header();
  }
}

void MathLoggerDriver::display_message(const std::string& str) {
  for (auto logger : math_loggers_) {
    logger->display_message(str);
  }
}

void MathLoggerDriver::display_message(const std::string& str,
                                       LogUtils::LOGLEVEL level,
                                       const std::string& context) {
  for (auto logger : math_loggers_) {
    logger->display_message(str, level, context);
  }
}

void MathLoggerDriver::PrintIterationSeparatorBegin() {
  for (auto logger : math_loggers_) {
    logger->PrintIterationSeparatorBegin();
  }
}

void MathLoggerDriver::PrintIterationSeparatorEnd() {
  for (auto logger : math_loggers_) {
    logger->PrintIterationSeparatorEnd();
  }
}

MathLoggerImplementation::MathLoggerImplementation(
    const BENDERSMETHOD& method, const std::filesystem::path& file_path,
    std::streamsize width, HEADERSTYPE type) {
  switch (method) {
    case BENDERSMETHOD::BENDERS:
      implementation_ =
          std::make_shared<MathLoggerBase>(file_path, width, type);
      break;
    case BENDERSMETHOD::BENDERS_OUTERLOOP:
      implementation_ =
          std::make_shared<MathLoggerBaseExternalLoop>(file_path, width, type);
      break;
    case BENDERSMETHOD::BENDERS_BY_BATCH:
      implementation_ =
          std::make_shared<MathLoggerBendersByBatch>(file_path, width, type);
      break;
    case BENDERSMETHOD::BENDERS_BY_BATCH_OUTERLOOP:
      implementation_ = std::make_shared<MathLoggerBendersByBatchExternalLoop>(
          file_path, width, type);
      break;

    default:
      break;
  }
}

MathLoggerImplementation::MathLoggerImplementation(const BENDERSMETHOD& method,
                                                   std::streamsize width,
                                                   HEADERSTYPE type) {
  switch (method) {
    case BENDERSMETHOD::BENDERS:
      implementation_ = std::make_shared<MathLoggerBase>(width, type);
      break;
    case BENDERSMETHOD::BENDERS_OUTERLOOP:
      implementation_ =
          std::make_shared<MathLoggerBaseExternalLoop>(width, type);
      break;
    case BENDERSMETHOD::BENDERS_BY_BATCH:
      implementation_ = std::make_shared<MathLoggerBendersByBatch>(width, type);
      break;
    case BENDERSMETHOD::BENDERS_BY_BATCH_OUTERLOOP:
      implementation_ =
          std::make_shared<MathLoggerBendersByBatchExternalLoop>(width, type);
      break;

    default:
      break;
  }
}

MathLoggerImplementation::MathLoggerImplementation(
    std::shared_ptr<MathLogger> implementation)
    : implementation_(std::move(implementation)) {}

void MathLoggerImplementation::display_message(const std::string& str) {
  implementation_->display_message(str);
}

void MathLoggerImplementation::display_message(const std::string& str,
                                               LogUtils::LOGLEVEL level,
                                               const std::string& context) {
  implementation_->display_message(str, level, context);
}
void MathLoggerImplementation::Print(const CurrentIterationData& data) {
  implementation_->Print(data);
}

void MathLoggerImplementation::PrintIterationSeparatorBegin() {
  implementation_->PrintIterationSeparatorBegin();
}

void MathLoggerImplementation::PrintIterationSeparatorEnd() {
  implementation_->PrintIterationSeparatorEnd();
}

void MathLoggerImplementation::setHeadersList() {
  implementation_->setHeadersList();
}

std::vector<std::string> MathLoggerImplementation::Headers() const {
  return implementation_->Headers();
}

LogDestination& MathLoggerImplementation::LogsDestination() {
  return implementation_->LogsDestination();
}
