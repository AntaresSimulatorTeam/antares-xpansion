#include "BendersMathLogger.h"

// #ifdef _WIN32
// #include <Windows.h>
// #endif

HeadersManager::HeadersManager(HEADERSTYPE type, const BENDERSMETHOD& method) {
  if (type == HEADERSTYPE::SHORT) {
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
    headers_list.push_back("IteTime");
    headers_list.push_back("MasterTime");
    // headers_list.push_back("SubPbCpuTime");
    headers_list.push_back("SPWallTime");
  } else {
    headers_list.push_back("ITERATION");
    headers_list.push_back("LB");
    if (method == BENDERSMETHOD::BENDERS) {
      headers_list.push_back("UB");
      headers_list.push_back("BESTUB");
      headers_list.push_back("ABSOLUTE GAP");
      headers_list.push_back("RELATIVE GAP");
    }
    headers_list.push_back("MINSIMPLEX");
    headers_list.push_back("MAXSIMPLEX");
    headers_list.push_back("NUMBER OF SUBPROBLEMS SOLVED");
    headers_list.push_back("CUMULATIVE NUMBER OF SUBPROBLEMS SOLVED ");
    headers_list.push_back("ITERATION TIME");
    headers_list.push_back("MASTER TIME");
    headers_list.push_back("SUB-PROBLEMS TIME (CPU)");
    headers_list.push_back("SUB-PROBLEMS TIME (WALL)");
    headers_list.push_back("TIME NOT DOING MASTER OR SUB-PROBLEMS (WALL)");
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
