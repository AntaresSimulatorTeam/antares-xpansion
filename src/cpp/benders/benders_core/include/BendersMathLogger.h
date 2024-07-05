#pragma once

#include <fstream>
#include <iostream>
#include <list>
#include <memory>

#include "BendersStructsDatas.h"
#include "ILogger.h"
#include "common.h"
const std::string MATHLOGGERCONTEXT = "Benders";

enum class HEADERSTYPE { SHORT, LONG };
struct HeadersManager {
  explicit HeadersManager(HEADERSTYPE type, const BENDERSMETHOD& method);

  HEADERSTYPE type_;
  BENDERSMETHOD method_;

  virtual std::vector<std::string> HeadersList();
};
struct HeadersManagerExternalLoop : HeadersManager {
  explicit HeadersManagerExternalLoop(HEADERSTYPE type,
                                      const BENDERSMETHOD& method);
  std::vector<std::string> HeadersList() override;
};

class LogDestination {
 public:
  explicit LogDestination(std::streamsize width = 40);
  explicit LogDestination(const std::filesystem::path& file_path,
                          std::streamsize width = 40);

  std::ostream& operator<<(std::ostream& (*function)(std::ostream&)) {
    return function(*stream_);
  }

  template <class T>
  std::ostream& operator<<(const T& obj);

 private:
  std::ofstream file_stream_;
  std::ostream* stream_;
  std::streamsize width_ = 40;
};
template <class T>
std::ostream& LogDestination::operator<<(const T& obj) {
  return (*stream_) << std::left << std::setw(width_) << obj;
}
void PrintBendersData(LogDestination& log_destination,
                      const CurrentIterationData& data, const HEADERSTYPE& type,
                      const BENDERSMETHOD& method);

void PrintExternalLoopData(LogDestination& log_destination,
                           const CurrentIterationData& data,
                           const HEADERSTYPE& type,
                           const BENDERSMETHOD& method);

struct MathLoggerBehaviour : public ILoggerXpansion {
  void write_header() {
    setHeadersList();
    for (const auto& header : Headers()) {
      LogsDestination() << header;
    }
    LogsDestination() << std::endl;
  }

  void display_message(const std::string& str) override {
    LogsDestination() << str << std::endl;
  }

  virtual void Print(const CurrentIterationData& data) = 0;
  virtual std::vector<std::string> Headers() const = 0;
  virtual LogDestination& LogsDestination() = 0;

  virtual void PrintIterationSeparatorBegin() override;
  virtual void PrintIterationSeparatorEnd() override;
  virtual void setHeadersList() = 0;
  virtual ~MathLoggerBehaviour() = default;
};

struct MathLogger : public MathLoggerBehaviour {
  explicit MathLogger(const std::filesystem::path& file_path,
                      std::streamsize width = 40,
                      HEADERSTYPE type = HEADERSTYPE::LONG)
      : log_destination_(file_path, width), type_(type) {}

  explicit MathLogger(std::streamsize width = 40,
                      HEADERSTYPE type = HEADERSTYPE::SHORT)
      : log_destination_(width), type_(type) {}

  virtual void Print(const CurrentIterationData& data) = 0;
  std::vector<std::string> Headers() const override { return headers_; }
  virtual LogDestination& LogsDestination() { return log_destination_; }
  virtual void setHeadersList() = 0;
  HEADERSTYPE HeadersType() const { return type_; }
  virtual ~MathLogger() = default;

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
  virtual ~MathLoggerBase() = default;
};

struct MathLoggerBaseExternalLoop : public MathLoggerBase {
  using MathLoggerBase::MathLoggerBase;
  void Print(const CurrentIterationData& data) override;
  void setHeadersList() override;
  virtual ~MathLoggerBaseExternalLoop() = default;
};

struct MathLoggerBendersByBatch : public MathLogger {
  using MathLogger::MathLogger;
  void Print(const CurrentIterationData& data) override;
  void setHeadersList() override;
  virtual ~MathLoggerBendersByBatch() = default;
};
struct MathLoggerBendersByBatchExternalLoop : public MathLoggerBendersByBatch {
  using MathLoggerBendersByBatch::MathLoggerBendersByBatch;
  void Print(const CurrentIterationData& data) override;
  void setHeadersList() override;
  virtual ~MathLoggerBendersByBatchExternalLoop() = default;
};

template <class T>
struct MathLoggerExternalLoopSpecific : public MathLogger {
  explicit MathLoggerExternalLoopSpecific(
      const std::filesystem::path& file_path,
      const std::vector<std::string>& headers,
      T OuterLoopCurrentIterationData::*ptr)
      : MathLogger(file_path), ptr_(ptr) {
    headers_.push_back("Outer loop");
    headers_.insert(headers_.end(), headers.begin(), headers.end());
  }

  void setHeadersList() override;
  void Print(const CurrentIterationData& data) override;

  virtual ~MathLoggerExternalLoopSpecific() = default;

 private:
  std::vector<std::string> headers_;
  T OuterLoopCurrentIterationData::*ptr_;
};

class MathLoggerImplementation : public MathLoggerBehaviour {
 public:
  explicit MathLoggerImplementation(const BENDERSMETHOD& method,
                                    const std::filesystem::path& file_path,
                                    std::streamsize width = 40,
                                    HEADERSTYPE type = HEADERSTYPE::LONG);
  explicit MathLoggerImplementation(const BENDERSMETHOD& method,
                                    std::streamsize width = 40,
                                    HEADERSTYPE type = HEADERSTYPE::LONG);
  explicit MathLoggerImplementation(std::shared_ptr<MathLogger> implementation);

  void Print(const CurrentIterationData& data) { implementation_->Print(data); }

  void PrintIterationSeparatorBegin() override {
    implementation_->PrintIterationSeparatorBegin();
  }
  void PrintIterationSeparatorEnd() override {
    implementation_->PrintIterationSeparatorEnd();
  }
  virtual ~MathLoggerImplementation() = default;

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

class MathLoggerDriver : public ILoggerXpansion {
 public:
  MathLoggerDriver() = default;
  void write_header();
  void display_message(const std::string& str) override;
  void add_logger(std::shared_ptr<MathLoggerImplementation> logger);
  template <class T>
  void add_logger(const std::filesystem::path& file_path,
                  const std::vector<std::string>& headers,
                  T OuterLoopCurrentIterationData::*t);
  void Print(const CurrentIterationData& data);
  virtual void PrintIterationSeparatorBegin() override;
  virtual void PrintIterationSeparatorEnd() override;
  virtual ~MathLoggerDriver() = default;

 private:
  std::vector<std::shared_ptr<MathLoggerImplementation>> math_loggers_;
};

template <class T>
void MathLoggerDriver::add_logger(const std::filesystem::path& file_path,
                                  const std::vector<std::string>& headers,
                                  T OuterLoopCurrentIterationData::*t) {
  auto impl = std::make_shared<MathLoggerExternalLoopSpecific<T>>(file_path,
                                                                  headers, t);
  add_logger(std::make_shared<MathLoggerImplementation>(impl));
}

template <class T>
void MathLoggerExternalLoopSpecific<T>::setHeadersList() {
  MathLogger::setHeadersList(headers_);
}
template <class T>
void MathLoggerExternalLoopSpecific<T>::Print(
    const CurrentIterationData& data) {
  LogsDestination() << data.outer_loop_current_iteration_data.benders_num_run;
  for (const auto& t : data.outer_loop_current_iteration_data.*ptr_) {
    LogsDestination() << std::scientific << std::setprecision(10) << t;
  }
  LogsDestination() << std::endl;
}