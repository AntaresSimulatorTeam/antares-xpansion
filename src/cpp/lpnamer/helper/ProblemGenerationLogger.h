#ifndef __PROBLEMGENERATIONLOGGER_H__
#define __PROBLEMGENERATIONLOGGER_H__
#include <chrono>
// #include <cstdio>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <list>
#include <memory>
#include <ostream>
#include <set>
#include <string>
inline std::string LogLocationToStr(int line, const char* file,
                                    const char* func) {
  return std::string("This is line ") + std::to_string(line) + " of file " +
         file + " (function " + func + ")\n";
}
#define LOGLOCATION LogLocationToStr(__LINE__, __FILE__, __func__)
namespace ProblemGenerationLog {

enum class LOGLEVEL { NONE, TRACE, DEBUG, INFO, WARNING, ERROR, FATAL };
enum class LOGGERTYPE { NONE, FILE, CONSOLE };
std::string LogLevelToStr(const LOGLEVEL log_level);
class ProblemGenerationILogger {
 public:
  virtual ~ProblemGenerationILogger() = default;
  virtual void DisplayMessage(const std::string& message) = 0;
  virtual std::ostream& GetOstreamObject() = 0;
  LOGGERTYPE Type() const { return type_; }

 protected:
  void SetType(const LOGGERTYPE& type) { type_ = type; }

 private:
  LOGGERTYPE type_ = LOGGERTYPE::NONE;
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
  std::string prefix_;
  LOGLEVEL log_level_;

 public:
  explicit ProblemGenerationLogger(const LOGLEVEL log_level)
      : prefix_(LogLevelToStr(log_level)), log_level_(log_level) {}
  ~ProblemGenerationLogger() = default;

  void AddLogger(const ProblemGenerationILoggerSharedPointer& logger);
  void DisplayMessage(const std::string& message) const;
  void DisplayMessage(const std::string& message,
                      const LOGLEVEL log_level) const;
  void setLogLevel(const LOGLEVEL log_level);

  std::string PrefixMessage() const { return PrefixMessage(log_level_); }
  std::string PrefixMessage(const LOGLEVEL&) const;
  ProblemGenerationLogger& operator()(const LOGLEVEL log_level) {
    return (*this) << log_level;
  }
  ProblemGenerationLogger& operator()() { return (*this) << PrefixMessage(); }

  ProblemGenerationLogger& operator<<(std::ostream& (*f)(std::ostream&));
  ProblemGenerationLogger& operator<<(
      const ProblemGenerationLoggerSharedPointer logger) {
    return (*logger);
  }
  ProblemGenerationLogger& operator<<(const LOGLEVEL log_level);

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

}  // namespace ProblemGenerationLog
#endif  //__PROBLEMGENERATIONLOGGER_H__