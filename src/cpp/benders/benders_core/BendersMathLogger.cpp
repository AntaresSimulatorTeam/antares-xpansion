#include "BendersMathLogger.h"

#include "LogUtils.h"
#include "LoggerUtils.h"

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
  headers_list.push_back("Criterion value");
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
    case BENDERSMETHOD::BENDERS_EXTERNAL_LOOP:
      implementation_ =
          std::make_shared<MathLoggerBaseExternalLoop>(file_path, width, type);
      break;
    case BENDERSMETHOD::BENDERS_BY_BATCH:
      implementation_ =
          std::make_shared<MathLoggerBendersByBatch>(file_path, width, type);
      break;
    case BENDERSMETHOD::BENDERS_BY_BATCH_EXTERNAL_LOOP:
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
    case BENDERSMETHOD::BENDERS_EXTERNAL_LOOP:
      implementation_ =
          std::make_shared<MathLoggerBaseExternalLoop>(width, type);
      break;
    case BENDERSMETHOD::BENDERS_BY_BATCH:
      implementation_ = std::make_shared<MathLoggerBendersByBatch>(width, type);
      break;
    case BENDERSMETHOD::BENDERS_BY_BATCH_EXTERNAL_LOOP:
      implementation_ =
          std::make_shared<MathLoggerBendersByBatchExternalLoop>(width, type);
      break;

    default:
      break;
  }
}