#ifndef __PROBLEMGENERATIONLOGGER_H__
#define __PROBLEMGENERATIONLOGGER_H__
#include <filesystem>
#include <fstream>
#include <iostream>
#include <list>
#include <memory>
#include <ostream>
#include <string>

namespace ProblemGenerationLog {
enum class LOGLEVEL { NONE, DEBUG, INFO, WARNING, ERROR, FATAL };
std::string LogLevelToStr(const LOGLEVEL logLevel) {
  switch (logLevel) {
    case LOGLEVEL::DEBUG:
      return "<Debug> ";
      break;
    case LOGLEVEL::INFO:
      return "<Info> ";
    case LOGLEVEL::WARNING:
      return "<Warning> ";
    case LOGLEVEL::ERROR:
      return "<Error> ";
    case LOGLEVEL::FATAL:
      return "<Fatal> ";
    default:
      return "";
  }
}
class ProblemGenerationILogger {
 private:
  std::string prefix_;
  LOGLEVEL logLevel_;

 public:
  virtual ~ProblemGenerationILogger() = default;
  ProblemGenerationILogger(const LOGLEVEL logLevel)
      : prefix_(LogLevelToStr(logLevel)), logLevel_(logLevel) {}
  virtual void DisplayMessage(const std::string& message) = 0;
  void setLogLevel(const LOGLEVEL logLevel) {
    logLevel_ = logLevel;
    prefix_ = LogLevelToStr(logLevel_);
  }

 protected:
  std::string Prefix() const { return prefix_; }
};
using ProblemGenerationILoggerSharedPointer =
    std::shared_ptr<ProblemGenerationILogger>;
class ProblemGenerationFileLogger : public ProblemGenerationILogger {
 private:
  std::filesystem::path logFilePath_;
  std::ofstream logFile_;

 public:
  virtual ~ProblemGenerationFileLogger() = default;
  ProblemGenerationFileLogger(const LOGLEVEL logLevel,
                              const std::filesystem::path& logFilePath);
  void DisplayMessage(const std::string& message) override;
};

class ProblemGenerationOstreamLogger : public ProblemGenerationILogger {
 private:
  std::ostream& stream_;

 public:
  virtual ~ProblemGenerationOstreamLogger() = default;
  ProblemGenerationOstreamLogger(const LOGLEVEL logLevel, std::ostream& stream);
  void DisplayMessage(const std::string& message) override;
};

class ProblemGenerationLogger : public ProblemGenerationILogger {
 public:
  void AddLogger(const ProblemGenerationILoggerSharedPointer& logger) {
    loggers_.push_back(logger);
  }
  void DisplayMessage(const std::string& message) override;

 private:
  std::list<ProblemGenerationILoggerSharedPointer> loggers_;
};
}  // namespace ProblemGenerationLog
#endif  //__PROBLEMGENERATIONLOGGER_H__