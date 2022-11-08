#include "ProblemGenerationLogger.h"

#include "Clock.h"

namespace ProblemGenerationLog {

std::string LogLevelToStr(const LOGLEVEL log_level) {
  switch (log_level) {
    case LOGLEVEL::TRACE:
      return "<Trace> ";
    case LOGLEVEL::DEBUG:
      return "<Debug> ";
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

ProblemGenerationFileLogger::ProblemGenerationFileLogger(
    const std::filesystem::path& logFilePath)
    : logFilePath_(logFilePath) {
  SetType(LOGGERTYPE::FILE);
  logFile_.open(logFilePath_, std::ofstream::out | std::ofstream::app);
  if (logFile_.fail()) {
    std::cerr << "ProblemGenerationFileLogger: Invalid file ("
              << logFilePath_.string() << ") passed as parameter" << std::endl;
  }
}
void ProblemGenerationFileLogger::DisplayMessage(const std::string& message) {
  logFile_ << message << std::endl;
  logFile_.flush();
}

ProblemGenerationOstreamLogger::ProblemGenerationOstreamLogger(
    std::ostream& stream)
    : stream_(stream) {
  SetType(LOGGERTYPE::CONSOLE);
  if (stream_.fail()) {
    std::cerr
        << "ProblemGenerationOstreamLogger: Invalid stream passed as parameter"
        << std::endl;
  }
}
void ProblemGenerationOstreamLogger::DisplayMessage(
    const std::string& message) {
  stream_ << message << std::endl;
}
void ProblemGenerationLogger::AddLogger(
    const ProblemGenerationILoggerSharedPointer& logger) {
  loggers_.push_back(logger);
  try_to_add_logger_to_enabled_list(logger);
}
void ProblemGenerationLogger::DisplayMessage(const std::string& message) const {
  for (const auto& logger : enabled_loggers_) {
    logger->DisplayMessage(message);
  }
}
void ProblemGenerationLogger::DisplayMessage(const std::string& message,
                                             const LOGLEVEL log_level) const {
  for (const auto& logger : enabled_loggers_) {
    logger->DisplayMessage(LogLevelToStr(log_level));
    logger->DisplayMessage(message);
  }
}
void ProblemGenerationLogger::setLogLevel(const LOGLEVEL log_level) {
  log_level_ = log_level;
  prefix_ = LogLevelToStr(log_level_);
  update_enabled_logger();
}
void ProblemGenerationLogger::update_enabled_logger() {
  enabled_loggers_.clear();
  for (const auto& logger : loggers_) {
    try_to_add_logger_to_enabled_list(logger);
  }
}
bool ProblemGenerationLogger::try_to_add_logger_to_enabled_list(
    const ProblemGenerationILoggerSharedPointer& logger) {
  if ((logger->Type() == LOGGERTYPE::CONSOLE && log_level_ != LOGLEVEL::DEBUG &&
       log_level_ != LOGLEVEL::TRACE) ||
      logger->Type() == LOGGERTYPE::FILE) {
    enabled_loggers_.insert(logger);
    return true;
  }
  return false;
}
std::string ProblemGenerationLogger::PrefixMessage(
    const LOGLEVEL& log_level) const {
  return LogLevelToStr(log_level) + clock_utils::timeToStr(std::time(nullptr)) +
         ": ";
}
ProblemGenerationLogger& ProblemGenerationLogger::operator<<(
    const LOGLEVEL log_level) {
  setLogLevel(log_level);
  for (const auto& subLogger : enabled_loggers_) {
    subLogger->GetOstreamObject() << PrefixMessage(log_level);
  }
  return *this;
}

ProblemGenerationLogger& ProblemGenerationLogger::operator<<(
    std::ostream& (*f)(std::ostream&)) {
  for (const auto& subLogger : enabled_loggers_) {
    subLogger->GetOstreamObject() << f;
  }
  return *this;
}

}  // namespace ProblemGenerationLog