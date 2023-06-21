#ifndef __PROBLEMGENERATIONLOGGER_H__
#define __PROBLEMGENERATIONLOGGER_H__
#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <list>
#include <memory>
#include <ostream>
#include <set>
#include <string>

#include "LogUtils.h"
#include "LoggerUtils.h"
namespace ProblemGenerationLog {

class ProblemGenerationILogger {
 public:
  virtual ~ProblemGenerationILogger() = default;
  virtual void DisplayMessage(const std::string& message) = 0;
  virtual std::ostream& GetOstreamObject() = 0;
  LogUtils::LOGGERTYPE Type() const { return type_; }

 protected:
  void SetType(const LogUtils::LOGGERTYPE& type) { type_ = type; }

 private:
  LogUtils::LOGGERTYPE type_ = LogUtils::LOGGERTYPE::NONE;
};
using ProblemGenerationILoggerSharedPointer =
    std::shared_ptr<ProblemGenerationILogger>;
class ProblemGenerationFileLogger : public ProblemGenerationILogger {
 private:
  std::filesystem::path logFilePath_;
  std::ofstream logFile_;

 public:
  ~ProblemGenerationFileLogger() override { logFile_.close(); }
  explicit ProblemGenerationFileLogger(
      const std::filesystem::path& logFilePath);
  void DisplayMessage(const std::string& message) override;
  std::ostream& GetOstreamObject() override { return logFile_; }
};

class ProblemGenerationOstreamLogger : public ProblemGenerationILogger {
 private:
  std::ostream& stream_;

 public:
  ~ProblemGenerationOstreamLogger() override = default;
  explicit ProblemGenerationOstreamLogger(std::ostream& stream);
  void DisplayMessage(const std::string& message) override;
  std::ostream& GetOstreamObject() override { return stream_; }
};

class ProblemGenerationLogger;
using ProblemGenerationLoggerSharedPointer =
    std::shared_ptr<ProblemGenerationLogger>;

class ProblemGenerationLogger {
 private:
  LogUtils::LOGLEVEL log_level_;
  std::string context_ = "Unknown Context";

 public:
  explicit ProblemGenerationLogger(const LogUtils::LOGLEVEL log_level)
      : log_level_(log_level) {}
  ~ProblemGenerationLogger() = default;

  void AddLogger(const ProblemGenerationILoggerSharedPointer& logger);
  void DisplayMessage(const std::string& message) const;
  void DisplayMessage(const std::string& message,
                      const LogUtils::LOGLEVEL log_level) const;
  void setLogLevel(const LogUtils::LOGLEVEL log_level);
  void setContext(const std::string& context) { context_ = context; }

  ProblemGenerationLogger& operator()(const LogUtils::LOGLEVEL log_level) {
    return (*this) << log_level;
  }
  ProblemGenerationLogger& operator()() {
    return (*this) << PrefixMessage(log_level_, context_);
  }

  ProblemGenerationLogger& operator<<(std::ostream& (*f)(std::ostream&));
  ProblemGenerationLogger& operator<<(
      const ProblemGenerationLoggerSharedPointer logger) {
    return (*logger);
  }
  ProblemGenerationLogger& operator<<(const LogUtils::LOGLEVEL log_level);

  template <typename T>
  ProblemGenerationLogger& operator<<(T const& t);

 private:
  std::list<ProblemGenerationILoggerSharedPointer> loggers_;
  std::set<ProblemGenerationILoggerSharedPointer> enabled_loggers_;
  void update_enabled_logger();
  bool try_to_add_logger_to_enabled_list(
      const ProblemGenerationILoggerSharedPointer& logger);
};
template <typename T>
ProblemGenerationLogger& ProblemGenerationLogger::operator<<(T const& t) {
  for (const auto& subLogger : enabled_loggers_) {
    subLogger->GetOstreamObject() << t;
  }
  return *this;
}
ProblemGenerationLoggerSharedPointer BuildLogger(
    const std::filesystem::path& log_file_path, std::ostream& stream,
    const std::string& context);

}  // namespace ProblemGenerationLog
#endif  //__PROBLEMGENERATIONLOGGER_H__