#include "BendersMathLogger.h"

void MathLoggerDriver::add_logger(MathLogger* logger) {
  if (logger) {
    math_loggers_.push_back(logger);
  }
}

void MathLoggerDriver::Print(const MathLoggerData& data) {
  for (auto logger : math_loggers_) {
    logger->Print(data);
  }
}

void MathLoggerDriver::write_header() {
  for (auto logger : math_loggers_) {
    logger->write_header();
  }
}
