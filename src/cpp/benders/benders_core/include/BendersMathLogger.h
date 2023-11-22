#pragma once

#include <fstream>
#include <iostream>
#include <list>
#include <memory>
#include "BendersStructsDatas.h"
const std::string MATHLOGGERCONTEXT = "Benders";


class LogDestination {
 public:
  explicit LogDestination(std::ostream* stream) : stream_(stream) {}

  // for std::endl
  std::ostream& operator<<(std::ostream& (*function)(std::ostream&)) {
    // write obj to stream
    return function(*stream_);
  }

  //   // for std::endl
  //   std::ostream& operator<<(const std::_Smanip<std::streamsize>& smanip) {
  //     // write obj to stream
  //     return (*stream_) << smanip;
  //   }

  template <class T>
  std::ostream& operator<<(const T& obj);

 private:
  std::ostream* stream_;
};
template <class T>
std::ostream& LogDestination::operator<<(const T& obj) {
  // write obj to stream
  return (*stream_) << obj;
}
struct MathLogger {
  explicit MathLogger(std::ostream* stream) : log_destination(stream) {}
  explicit MathLogger() : log_destination(&std::cout) {}
  void write_header();
  void Print(const CurrentIterationData& data);
  LogDestination log_destination;
};

class MathLoggerDriver {
 public:
  MathLoggerDriver() = default;
  void write_header();
  void add_logger(std::shared_ptr<MathLogger> logger);
  void Print(const CurrentIterationData& data);

 private:
  std::list<std::shared_ptr<MathLogger>> math_loggers_;
};
