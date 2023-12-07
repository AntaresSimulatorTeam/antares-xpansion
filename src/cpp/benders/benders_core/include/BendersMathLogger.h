#pragma once

#include <fstream>
#include <iostream>
#include <list>
#include <memory>

#include "BendersStructsDatas.h"
#include "common.h"
const std::string MATHLOGGERCONTEXT = "Benders";

inline std::string Indent(int size) { return std::string(size, ' '); }
const std::string ITERATION = Indent(10) + "ITERATION";
const std::string LB = Indent(20) + "LB";
const std::string UB = Indent(20) + "UB";
const std::string BESTUB = Indent(20) + "BESTUB";
const std::string ABSOLUTE_GAP = Indent(15) + "ABSOLUTE GAP";
const std::string RELATIVE_GAP = Indent(15) + "RELATIVE GAP";
const std::string MINSIMPLEX = Indent(15) + "MINSIMPLEX";
const std::string MAXSIMPLEX = Indent(15) + "MAXSIMPLEX";
const std::string TIMEMASTER = Indent(15) + "TIMEMASTER";
const std::string SUB_PROBLEMS_TIME_CPU =
    Indent(15) + "SUB-PROBLEMS TIME (CPU)";
const std::string SUB_PROBLEMS_TIME_WALL =
    Indent(15) + "SUB-PROBLEMS TIME (WALL)";
const std::string TIME_NOT_DOING_MASTER_OR_SUB_PROBLEMS_WALL =
    Indent(15) + "TIME NOT DOING MASTER OR SUB-PROBLEMS (WALL)";
class LogDestination {
 public:
  explicit LogDestination(std::ostream* stream) : stream_(stream) {}

  // for std::endl
  std::ostream& operator<<(std::ostream& (*function)(std::ostream&)) {
    // write obj to stream
    return function(*stream_);
  }

  template <class T>
  std::ostream& operator<<(const T& obj);

 private:
  std::ostream* stream_;
};
template <class T>
std::ostream& LogDestination::operator<<(const T& obj) {
  // write obj to stream
  return (*stream_) << std::left << obj;
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
  explicit MathLogger(std::ostream* stream) : log_destination_(stream) {}
  explicit MathLogger() : log_destination_(&std::cout) {}
  virtual void Print(const CurrentIterationData& data) = 0;
  std::list<std::string> Headers() const override { return headers_; }
  virtual LogDestination& LogsDestination() { return log_destination_; }
  virtual void setHeadersList() = 0;

 protected:
  void setHeadersList(const std::list<std::string>& headers);

 private:
  std::list<std::string> headers_;
  LogDestination log_destination_;
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
                                    std::ostream* stream) {
    if (method == BENDERSMETHOD::BENDERS) {
      implementation_ = std::make_shared<MathLoggerBase>(stream);
    } else if (method == BENDERSMETHOD::BENDERSBYBATCH) {
      implementation_ = std::make_shared<MathLoggerBendersByBatch>(stream);
    }
    // else
  }
  explicit MathLoggerImplementation(const BENDERSMETHOD& method) {
    if (method == BENDERSMETHOD::BENDERS) {
      implementation_ = std::make_shared<MathLoggerBase>();
    } else if (method == BENDERSMETHOD::BENDERSBYBATCH) {
      implementation_ = std::make_shared<MathLoggerBendersByBatch>();
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
