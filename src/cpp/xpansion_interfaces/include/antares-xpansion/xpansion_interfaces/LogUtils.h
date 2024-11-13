#pragma once
#include <stdexcept>
#include <string>
namespace LogUtils {
inline std::string LogLocationToStr(int line, const char* file,
                                    const char* func) {
  return std::string("Logged from function '") + func + "' in file '" + file +
         "' at line " + std::to_string(line) + ".\n";
}

template <typename T>
class XpansionError : public T {
 public:
  explicit XpansionError(const std::string& err_message,
                         const std::string& log_location)
      : T(log_location + err_message), err_message_(err_message) {}
  explicit XpansionError(const std::string& prefix,
                         const std::string& err_message,
                         const std::string& log_location)
      : T(log_location + prefix + err_message), err_message_(err_message) {}

 public:
  std::string ErrorMessage() const { return err_message_; }

 private:
  const std::string err_message_;
};

enum class LOGLEVEL { NONE, TRACE, DEBUG, INFO, WARNING, ERR, FATAL };
enum class LOGGERTYPE { NONE, FILE, CONSOLE };
inline std::string LogLevelToStr(const LogUtils::LOGLEVEL log_level) {
  switch (log_level) {
    case LogUtils::LOGLEVEL::TRACE:
      return "TRACE";
    case LogUtils::LOGLEVEL::DEBUG:
      return "DEBUG";
    case LogUtils::LOGLEVEL::INFO:
      return "INFO";
    case LogUtils::LOGLEVEL::WARNING:
      return "WARNING";
    case LogUtils::LOGLEVEL::ERR:
      return "ERROR";
    case LogUtils::LOGLEVEL::FATAL:
      return "FATAL";
    default:
      return "";
  }
}
}  // namespace LogUtils

#define LOGLOCATION LogUtils::LogLocationToStr(__LINE__, __FILE__, __func__)
