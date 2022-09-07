#include "ProblemGenerationLogger.h"
namespace ProblemGenerationLog {

std::string LogLevelToStr(const LOGLEVEL logLevel) {
  switch (logLevel) {
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

void ProblemGenerationLogger::DisplayMessage(const std::string& message) {
  for (auto& logger : loggers_) {
    logger->DisplayMessage(message);
  }
}
void ProblemGenerationLogger::DisplayMessage(const std::string& message,
                                             const LOGLEVEL logLevel) {
  for (auto& logger : loggers_) {
    logger->DisplayMessage(LogLevelToStr(logLevel));
    logger->DisplayMessage(message);
  }
}

ProblemGenerationLogger& operator<<(ProblemGenerationLogger& logger,
                                    const LOGLEVEL logLevel) {
  for (auto& subLogger : logger.loggers_) {
    subLogger->GetOstreamObject() << LogLevelToStr(logLevel);
  }
  return logger;
}
ProblemGenerationLogger& operator<<(
    const ProblemGenerationLoggerSharedPointer& logger,
    const LOGLEVEL logLevel) {
  return (*logger) << logLevel;
}

ProblemGenerationLogger& operator<<(ProblemGenerationLogger& logger,
                                    std::ostream& (*f)(std::ostream&)) {
  for (auto& subLogger : logger.loggers_) {
    subLogger->GetOstreamObject() << f;
  }
  return logger;
}

ProblemGenerationLogger& operator<<(
    const ProblemGenerationLoggerSharedPointer& logger,
    std::ostream& (*f)(std::ostream&)) {
  return (*logger) << f;
}
}  // namespace ProblemGenerationLog