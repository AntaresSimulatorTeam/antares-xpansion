
#pragma once

#include <filesystem>
#include <fstream>

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
  double alpha;
  const std::string CONTEXT = "Benders";
};

class LogDestination {
 public:
  explicit LogDestination(std::ostream* stream) : stream_(stream) {}
  template <class T>
  std::ostream& operator<<(const T& obj) {
    // write obj to stream
    return (*stream_) << T;
  }

 private:
  std::ostream* stream_;
};

struct MathLogger {
  explicit MathLogger(std::ostream* stream) : log_destination(stream) {}
  void write_header();

  MathLoggerData data;
  LogDestination log_destination;
};

class MathLoggerFile : public MathLogger {
 public:
  explicit MathLoggerFile(const std::filesystem::path& log_file);

 private:
  std::ofstream file_stream_;
};