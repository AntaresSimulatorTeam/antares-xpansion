#include "BendersMathLogger.h"

#include "LogUtils.h"

HeadersManager::HeadersManager(HEADERSTYPE type, const BENDERSMETHOD& method) {
  headers_list.push_back("ITE");
  headers_list.push_back("LB");
  if (method == BENDERSMETHOD::BENDERS) {
    headers_list.push_back("UB");
    headers_list.push_back("BESTUB");
    headers_list.push_back("AGAP");
    headers_list.push_back("RGAP");
  }
  headers_list.push_back("MinSpx");
  headers_list.push_back("MaxSpx");
  if (method == BENDERSMETHOD::BENDERSBYBATCH) {
    headers_list.push_back("NbSubPbSolv");
  }
  if (type == HEADERSTYPE::LONG) {
    headers_list.push_back("CumulNbSubPbSolv");
  }
  headers_list.push_back("IteTime");
  headers_list.push_back("MasterTime");
  headers_list.push_back("SPWallTime");
  if (type == HEADERSTYPE::LONG) {
    headers_list.push_back("SPCpuTime");
    headers_list.push_back("NotSolvingWallTime");
  }
}

LogDestination::LogDestination(std::ostream* stream, std::streamsize width)
    : stream_(stream), width_(width) {
  if (!stream) {
    std::cerr << "the stream ptr  (std::ostream*) is null, &std::cout will be "
                 "used!\n";
    stream_ = &std::cout;
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
