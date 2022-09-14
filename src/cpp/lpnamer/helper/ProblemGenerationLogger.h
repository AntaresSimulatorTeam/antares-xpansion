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
#include <string>

namespace ProblemGenerationLog {
std::string GetTime();
enum class LOGLEVEL { NONE, TRACE, DEBUG, INFO, WARNING, ERROR, FATAL };
std::string LogLevelToStr(const LOGLEVEL logLevel);
class ProblemGenerationILogger {
 public:
  virtual ~ProblemGenerationILogger() = default;
  virtual void DisplayMessage(const std::string& message) = 0;
  virtual std::ostream& GetOstreamObject() = 0;
};
using ProblemGenerationILoggerSharedPointer =
    std::shared_ptr<ProblemGenerationILogger>;
class ProblemGenerationFileLogger : public ProblemGenerationILogger {
 private:
  std::filesystem::path logFilePath_;
  std::ofstream logFile_;

 public:
  virtual ~ProblemGenerationFileLogger() { logFile_.close(); }
  ProblemGenerationFileLogger(const std::filesystem::path& logFilePath);
  void DisplayMessage(const std::string& message) override;
  std::ostream& GetOstreamObject() { return logFile_; }
};

class ProblemGenerationOstreamLogger : public ProblemGenerationILogger {
 private:
  std::ostream& stream_;

 public:
  virtual ~ProblemGenerationOstreamLogger() = default;
  ProblemGenerationOstreamLogger(std::ostream& stream);
  void DisplayMessage(const std::string& message) override;
  std::ostream& GetOstreamObject() { return stream_; }
};

class ProblemGenerationLogger;
using ProblemGenerationLoggerSharedPointer =
    std::shared_ptr<ProblemGenerationLogger>;

class ProblemGenerationLogger {
 private:
  std::string prefix_;
  LOGLEVEL logLevel_;

 public:
  ProblemGenerationLogger(const LOGLEVEL logLevel)
      : prefix_(LogLevelToStr(logLevel)), logLevel_(logLevel) {}
  ~ProblemGenerationLogger() = default;

  void AddLogger(const ProblemGenerationILoggerSharedPointer& logger) {
    loggers_.push_back(logger);
  }
  void DisplayMessage(const std::string& message)const;
  void DisplayMessage(const std::string& message, const LOGLEVEL logLevel)const;
  void setLogLevel(const LOGLEVEL logLevel) {
    logLevel_ = logLevel;
    prefix_ = LogLevelToStr(logLevel_);
  }
  std::string PrefixMessage() const { return PrefixMessage(logLevel_); }
  std::string PrefixMessage(const LOGLEVEL&) const;
  ProblemGenerationLogger& operator()(const LOGLEVEL logLevel) {
    return (*this) << logLevel;
  }
  ProblemGenerationLogger& operator()() { return (*this) << PrefixMessage(); }

  ProblemGenerationLogger& operator<<(std::ostream& (*f)(std::ostream&));
  ProblemGenerationLogger& operator<<(
      const ProblemGenerationLoggerSharedPointer logger) {
    return (*logger);
  }
  ProblemGenerationLogger& operator<<(const LOGLEVEL logLevel);

  template <typename T>
  ProblemGenerationLogger& operator<<(T const& t);

 private:
  std::list<ProblemGenerationILoggerSharedPointer> loggers_;
};
template <typename T>
ProblemGenerationLogger& ProblemGenerationLogger::operator<<(T const& t) {
  for (const auto& subLogger : loggers_) {
    subLogger->GetOstreamObject() << t;
  }
  return *this;
}

}  // namespace ProblemGenerationLog
#endif  //__PROBLEMGENERATIONLOGGER_H__