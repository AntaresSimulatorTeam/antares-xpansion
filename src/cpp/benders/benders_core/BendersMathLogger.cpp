#include "BendersMathLogger.h"

// #ifdef _WIN32
// #include <Windows.h>
// #endif

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
  // _COORD coordinates;
  // coordinates.X = 1000;
  // coordinates.Y = 1000;

  // if (0 == SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE),
  //                                     coordinates)) {
  //   std::cout << "could not resize the console screen\n";
  //   // return -1;
  // }
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
