#pragma once

#include <fstream>
#include <iostream>
#include <list>
#include <memory>

#include "BendersStructsDatas.h"
#include "ILogger.h"
#include "common.h"
const std::string MATHLOGGERCONTEXT = "Benders";

inline std::string Indent(int size) { return std::string(size, ' '); }
struct Header {
  std::string label;
  int pos;
};

enum class HEADERSTYPE { SHORT, LONG };
struct HeadersManager {
  explicit HeadersManager(HEADERSTYPE type, const BENDERSMETHOD& method);
  std::vector<std::string> headers_list;

  // Header ITERATION;
  // Header LB;
  // Header UB;
  // Header BESTUB;
  // Header ABSOLUTE_GAP;
  // Header RELATIVE_GAP;
  // Header MINSIMPLEX;
  // Header MAXSIMPLEX;
  // Header NUMBER_OF_SUBPROBLEM_SOLVED;
  // Header CUMULATIVE_NUMBER_OF_SUBPROBLEM_SOLVED;
  // Header ITE_TIME;
  // Header TIMEMASTER;
  // Header SUB_PROBLEMS_TIME_CPU;
  // Header SUB_PROBLEMS_TIME_WALL;
  // Header TIME_NOT_DOING_MASTER_OR_SUB_PROBLEMS_WALL;
  // const std::string BATCH_SIZE = "BATCH SIZE";
  // const std::string SEPARATION_PARAMETER = "SEPARATION PARAMETER";
  //  private:
  //   void SetHeader(Header& header, const std::string& label);
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

struct MathLoggerBehaviour : public ILoggerBenders {
  void write_header() {
    setHeadersList();
    for (const auto& header : Headers()) {
      LogsDestination() << header;
    }
    LogsDestination() << std::endl;
  }

  virtual void display_message(const std::string& str) {
    LogsDestination() << str;
  }

  virtual void Print(const CurrentIterationData& data) = 0;
  virtual std::vector<std::string> Headers() const = 0;
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
  std::vector<std::string> Headers() const override { return headers_; }
  virtual LogDestination& LogsDestination() { return log_destination_; }
  virtual void setHeadersList() = 0;
  HEADERSTYPE HeadersType() const { return type_; }

 protected:
  void setHeadersList(const std::vector<std::string>& headers);

 private:
  std::vector<std::string> headers_;
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
  std::vector<std::string> Headers() const override {
    return implementation_->Headers();
  }
  virtual LogDestination& LogsDestination() {
    return implementation_->LogsDestination();
  }

 private:
  std::shared_ptr<MathLogger> implementation_;
};

class MathLoggerDriver : public ILoggerBenders {
 public:
  MathLoggerDriver() = default;
  void write_header();
  void display_message(const std::string& str) override;
  void add_logger(std::shared_ptr<MathLoggerImplementation> logger);
  void Print(const CurrentIterationData& data);

 private:
  std::vector<std::shared_ptr<MathLoggerImplementation>> math_loggers_;
};
