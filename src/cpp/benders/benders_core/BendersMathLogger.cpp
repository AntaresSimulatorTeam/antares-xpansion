#include "BendersMathLogger.h"

#include "LogUtils.h"
#include "LoggerUtils.h"

HeadersManager::HeadersManager(HEADERSTYPE type, const BENDERSMETHOD& method) {
  headers_list.push_back("Ite");
  headers_list.push_back("Lb");
  if (method == BENDERSMETHOD::BENDERS) {
    headers_list.push_back("Ub");
    headers_list.push_back("BestUb");
    headers_list.push_back("AbsGap");
    headers_list.push_back("RelGap");
  }
  headers_list.push_back("MinSpx");
  headers_list.push_back("MaxSpx");

  if (type == HEADERSTYPE::LONG || method == BENDERSMETHOD::BENDERSBYBATCH) {
    headers_list.push_back("NbSubPbSolv");
  }

  if (type == HEADERSTYPE::LONG) {
    headers_list.push_back("CumulNbSubPbSolv");
  }
  headers_list.push_back("IteTime (s)");
  headers_list.push_back("MasterTime (s)");
  headers_list.push_back("SPWallTime (s)");
  if (type == HEADERSTYPE::LONG) {
    headers_list.push_back("SPCpuTime (s)");
    headers_list.push_back("NotSolvingWallTime (s)");
  }
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
