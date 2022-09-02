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
std::string LogLevelToStr(const LOGLEVEL logLevel);
class ProblemGenerationILogger {
 private:
  std::string prefix_;
  LOGLEVEL logLevel_;

 public:
  virtual ~ProblemGenerationILogger() = default;
  ProblemGenerationILogger(const LOGLEVEL logLevel)
      : prefix_(LogLevelToStr(logLevel)), logLevel_(logLevel) {}
  virtual void DisplayMessage(const std::string& message) = 0;
  virtual void DisplayMessage(const std::string& message,
                              const LOGLEVEL logLevel) = 0;
  void setLogLevel(const LOGLEVEL logLevel) {
    logLevel_ = logLevel;
    prefix_ = LogLevelToStr(logLevel_);
  }
  virtual std::ostream& GetOstreamObject() = 0;

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
  void DisplayMessage(const std::string& message,
                      const LOGLEVEL logLevel) override;
  void DisplayMessage(const std::string& message) override;
  std::ostream& GetOstreamObject() { return logFile_; }
  //   template <typename T>
  //   friend ProblemGenerationFileLogger&
  //   operator<<(ProblemGenerationFileLogger& h,
  //                                                  T const& t);

  //   friend ProblemGenerationFileLogger& operator<<(
  //       ProblemGenerationFileLogger& h, std::ostream& (*f)(std::ostream&));
};

class ProblemGenerationOstreamLogger : public ProblemGenerationILogger {
 private:
  std::ostream& stream_;

 public:
  virtual ~ProblemGenerationOstreamLogger() = default;
  ProblemGenerationOstreamLogger(const LOGLEVEL logLevel, std::ostream& stream);
  void DisplayMessage(const std::string& message) override;
  void DisplayMessage(const std::string& message,
                      const LOGLEVEL logLevel) override;
  std::ostream& GetOstreamObject() { return stream_; }

  //   template <typename T>
  //   friend ProblemGenerationOstreamLogger& operator<<(
  //       ProblemGenerationOstreamLogger& h, T const& t);

  //   friend ProblemGenerationOstreamLogger& operator<<(
  //       ProblemGenerationOstreamLogger& h, std::ostream&
  //       (*f)(std::ostream&));
};

class ProblemGenerationLogger;

ProblemGenerationLogger& operator<<(ProblemGenerationLogger& h,
                                    std::ostream& (*f)(std::ostream&));
ProblemGenerationLogger& operator<<(ProblemGenerationLogger& h,
                                    const LOGLEVEL logLevel);
template <typename T>
ProblemGenerationLogger& operator<<(ProblemGenerationLogger& h, T const& t);

class ProblemGenerationLogger {
 public:
  ProblemGenerationLogger() = default;
  void AddLogger(const ProblemGenerationILoggerSharedPointer& logger) {
    loggers_.push_back(logger);
  }
  void DisplayMessage(const std::string& message);
  void DisplayMessage(const std::string& message, const LOGLEVEL logLevel);

  friend ProblemGenerationLogger& operator<<(ProblemGenerationLogger& h,
                                             std::ostream& (*f)(std::ostream&));
  friend ProblemGenerationLogger& operator<<(ProblemGenerationLogger& h,
                                             const LOGLEVEL logLevel);

  template <typename T>
  friend ProblemGenerationLogger& operator<<(ProblemGenerationLogger& h,
                                             T const& t);

 private:
  std::list<ProblemGenerationILoggerSharedPointer> loggers_;
};

// template <typename Logger, typename T>
// Logger& operator<<(Logger& h, T const& t);

// template <typename Logger>
// Logger& operator<<(Logger& h, std::ostream& (*f)(std::ostream&));

// template <typename T>
// ProblemGenerationOstreamLogger& operator<<(
//     ProblemGenerationOstreamLogger& logger, const LOGLEVEL logLevel) {
//   logger.stream_ << LogLevelToStr(logLevel);
//   return logger;
// }
// template <typename T>
// ProblemGenerationOstreamLogger& operator<<(
//     ProblemGenerationOstreamLogger& logger, T const& t) {
//   logger.stream_ << logger.Prefix() << t;
//   return logger;
// }

// ProblemGenerationOstreamLogger& operator<<(
//     ProblemGenerationOstreamLogger& logger, std::ostream&
//     (*f)(std::ostream&)) {
//   logger.stream_ << f;
//   return logger;
// }
// template <typename T>
// ProblemGenerationFileLogger& operator<<(ProblemGenerationFileLogger& logger,
//                                         const LOGLEVEL logLevel) {
//   logger.logFile_ << LogLevelToStr(logLevel);
//   logger.logFile_.flush();
//   return logger;
// }
// template <typename T>
// ProblemGenerationFileLogger& operator<<(ProblemGenerationFileLogger& logger,
//                                         T const& t) {
//   logger.logFile_ << logger.Prefix() << t;
//   logger.logFile_.flush();
//   return logger;
// }

// ProblemGenerationFileLogger& operator<<(ProblemGenerationFileLogger& logger,
//                                         std::ostream& (*f)(std::ostream&)) {
//   logger.logFile_ << f;
//   logger.logFile_.flush();
//   return logger;
// }

template <typename T>
ProblemGenerationLogger& operator<<(ProblemGenerationLogger& logger,
                                    T const& t) {
  for (auto& subLogger : logger.loggers_) {
    subLogger->GetOstreamObject() << t;
  }
  return logger;
}

}  // namespace ProblemGenerationLog
#endif  //__PROBLEMGENERATIONLOGGER_H__