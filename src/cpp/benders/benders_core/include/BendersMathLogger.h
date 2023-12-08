#pragma once

#include <fstream>
#include <iostream>
#include <list>
#include <memory>

#include "BendersStructsDatas.h"
#include "common.h"
const std::string MATHLOGGERCONTEXT = "Benders";

inline std::string Indent(int size) { return std::string(size, ' '); }

enum class HEADERSTYPE { SHORT, LONG };
struct HeadersManager {
  explicit HeadersManager(HEADERSTYPE type);

  std::string ITERATION;
  std::string LB = "LB";
  std::string UB = "UB";
  std::string BESTUB = "BESTUB";
  std::string ABSOLUTE_GAP = "ABSOLUTE GAP";
  std::string RELATIVE_GAP = "RELATIVE GAP";
  std::string MINSIMPLEX = "MINSIMPLEX";
  std::string MAXSIMPLEX = "MAXSIMPLEX";
  std::string NUMBER_OF_SUBPROBLEM_SOLVED = "NUMBER OF SUBPROBLEMS SOLVED";
  std::string CUMULATIVE_NUMBER_OF_SUBPROBLEM_SOLVED =
      "CUMULATIVE NUMBER OF SUBPROBLEMS SOLVED";
  std::string BENDERS_TIME = "BENDERS TIME";
  std::string TIMEMASTER = "MASTER TIME";
  std::string SUB_PROBLEMS_TIME_CPU = "SUB-PROBLEMS TIME (CPU)";
  std::string SUB_PROBLEMS_TIME_WALL = "SUB-PROBLEMS TIME (WALL)";
  std::string TIME_NOT_DOING_MASTER_OR_SUB_PROBLEMS_WALL =
      "TIME NOT DOING MASTER OR SUB-PROBLEMS (WALL)";
  // const std::string BATCH_SIZE = "BATCH SIZE";
  // const std::string SEPARATION_PARAMETER = "SEPARATION PARAMETER";
};
class LogDestination {
 public:
  explicit LogDestination(std::ostream* stream, std::streamsize width = 40);

  // for std::endl
  std::ostream& operator<<(std::ostream& (*function)(std::ostream&)) {
    // write obj to stream
    return function(*stream_);
  }

  template <class T>
  std::ostream& operator<<(const T& obj);

 private:
  std::ostream* stream_;
  std::streamsize width_ = 40;
};
template <class T>
std::ostream& LogDestination::operator<<(const T& obj) {
  // write obj to stream
  return (*stream_) << std::left << std::setw(width_) << obj;
}

struct MathLoggerBehaviour {
  void write_header() {
    setHeadersList();
    for (const auto& header : Headers()) {
      LogsDestination() << header;
    }
    LogsDestination() << std::endl;
  }
  virtual void Print(const CurrentIterationData& data) = 0;
  virtual std::list<std::string> Headers() const = 0;
  virtual LogDestination& LogsDestination() = 0;
  virtual void setHeadersList() = 0;
};

struct MathLogger : public MathLoggerBehaviour {
  explicit MathLogger(std::ostream* stream, std::streamsize width = 40,
                      HEADERSTYPE type = HEADERSTYPE::LONG)
      : log_destination_(stream, width), type_(type) {}

  explicit MathLogger(std::streamsize width = 40,
                      HEADERSTYPE type = HEADERSTYPE::LONG)
      : log_destination_(&std::cout, width), type_(type) {}
  virtual void Print(const CurrentIterationData& data) = 0;
  std::list<std::string> Headers() const override { return headers_; }
  virtual LogDestination& LogsDestination() { return log_destination_; }
  virtual void setHeadersList() = 0;
  HEADERSTYPE HeadersType() const { return type_; }

 protected:
  void setHeadersList(const std::list<std::string>& headers);

 private:
  std::list<std::string> headers_;
  LogDestination log_destination_;
  HEADERSTYPE type_;
};

struct MathLoggerBase : public MathLogger {
  using MathLogger::MathLogger;
  void Print(const CurrentIterationData& data) override;

  void setHeadersList() override;
};

struct MathLoggerBendersByBatch : public MathLogger {
  using MathLogger::MathLogger;
  void Print(const CurrentIterationData& data) override;
  void setHeadersList() override;
};

class MathLoggerImplementation : public MathLoggerBehaviour {
 public:
  explicit MathLoggerImplementation(const BENDERSMETHOD& method,
                                    std::ostream* stream,
                                    std::streamsize width = 40,
                                    HEADERSTYPE type = HEADERSTYPE::LONG) {
    if (method == BENDERSMETHOD::BENDERS) {
      implementation_ = std::make_shared<MathLoggerBase>(stream, width, type);
    } else if (method == BENDERSMETHOD::BENDERSBYBATCH) {
      implementation_ =
          std::make_shared<MathLoggerBendersByBatch>(stream, width, type);
    }
    // else
  }
  explicit MathLoggerImplementation(const BENDERSMETHOD& method,
                                    std::streamsize width = 40,
                                    HEADERSTYPE type = HEADERSTYPE::LONG) {
    if (method == BENDERSMETHOD::BENDERS) {
      implementation_ = std::make_shared<MathLoggerBase>(width, type);
    } else if (method == BENDERSMETHOD::BENDERSBYBATCH) {
      implementation_ = std::make_shared<MathLoggerBendersByBatch>(width, type);
    }
    // else }
  }

  void Print(const CurrentIterationData& data) { implementation_->Print(data); }

 protected:
  void setHeadersList() override { implementation_->setHeadersList(); }
  std::list<std::string> Headers() const override {
    return implementation_->Headers();
  }
  virtual LogDestination& LogsDestination() {
    return implementation_->LogsDestination();
  }

 private:
  std::shared_ptr<MathLogger> implementation_;
};

class MathLoggerDriver {
 public:
  MathLoggerDriver() = default;
  void write_header();

  void add_logger(std::shared_ptr<MathLoggerImplementation> logger);
  void Print(const CurrentIterationData& data);

 private:
  std::list<std::shared_ptr<MathLoggerImplementation>> math_loggers_;
};
