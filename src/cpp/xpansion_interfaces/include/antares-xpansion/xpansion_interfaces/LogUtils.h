#pragma once
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace LogUtils
{
inline std::string LogLocationToStr(int line, const char* file, const char* func)
{
    return std::string("Logged from function '") + func + "' in file '" + file + "' at line "
           + std::to_string(line) + ".\n";
}

template<typename T>
class XpansionError: public T
{
public:
    explicit XpansionError(const std::string& err_message, const std::string& log_location):
        T(log_location + err_message),
        err_message_(err_message)
    {
    }

    explicit XpansionError(const std::string& prefix,
                           const std::string& err_message,
                           const std::string& log_location):
        T(log_location + prefix + err_message),
        err_message_(err_message)
    {
    }

public:
    std::string ErrorMessage() const
    {
        return err_message_;
    }

private:
    const std::string err_message_;
};

enum class LOGLEVEL
{
    NONE,
    TRACE,
    DEBUG,
    INFO,
    WARNING,
    ERR,
    FATAL
};
enum class LOGGERTYPE
{
    NONE,
    FILE,
    CONSOLE
};

static const std::unordered_map<std::string, LogUtils::LOGLEVEL> LogStrMap = {
  {"NONE", LogUtils::LOGLEVEL::NONE},
  {"TRACE", LogUtils::LOGLEVEL::TRACE},
  {"DEBUG", LogUtils::LOGLEVEL::DEBUG},
  {"INFO", LogUtils::LOGLEVEL::INFO},
  {"WARNING", LogUtils::LOGLEVEL::WARNING},
  {"ERR", LogUtils::LOGLEVEL::ERR},
  {"FATAL", LogUtils::LOGLEVEL::FATAL}};

inline std::string LogLevelToStr(const LogUtils::LOGLEVEL log_level)
{
    switch (log_level)
    {
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

inline LogUtils::LOGLEVEL StrToLogLevel(std::string logStr)
{
    std::transform(logStr.begin(),
                   logStr.end(),
                   logStr.begin(),
                   [](unsigned char c) { return std::toupper(c); } // correct
    );
    auto it = LogUtils::LogStrMap.find(logStr);
    if (it != LogUtils::LogStrMap.end())
    {
        return it->second;
    }
    else
    {
        throw std::invalid_argument("Unknown log level: " + logStr);
    }
}
} // namespace LogUtils

#define LOGLOCATION LogUtils::LogLocationToStr(__LINE__, __FILE__, __func__)
