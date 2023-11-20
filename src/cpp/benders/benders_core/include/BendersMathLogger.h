#pragma once

#include <fstream>
#include <iostream>
#include <list>
const std::string MATHLOGGERCONTEXT = "Benders";

struct MathLoggerData {
  int iteration;
  double lower_bound;
  double upper_bound;
  double best_upper_bound;
  double optimality_gap;
  double relative_gap;
  int max_simplexiter;
  int min_simplexiter;
  int deletedcut;
  double time_master;
  double time_subproblems;
  // double alpha;
};

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
  void Print(const MathLoggerData& data);
  LogDestination log_destination;
};

class MathLoggerDriver {
 public:
  MathLoggerDriver() = default;
  void write_header();
  void MathLoggerDriver::add_logger(MathLogger* logger);
  void Print(const MathLoggerData& data);

 private:
  std::list<MathLogger*> math_loggers_;
};
