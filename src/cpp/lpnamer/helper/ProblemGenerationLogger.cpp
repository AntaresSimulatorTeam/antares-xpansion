#include "ProblemGenerationLogger.h"
namespace ProblemGenerationLog {

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

ProblemGenerationFileLogger::ProblemGenerationFileLogger(
    const LOGLEVEL logLevel, const std::filesystem::path& logFilePath)
    : ProblemGenerationILogger(logLevel), logFilePath_(logFilePath) {
  logFile_.open(logFilePath_, std::ofstream::out | std::ofstream::app);
  if (logFile_.fail()) {
    std::cerr << "ProblemGenerationFileLogger: Invalid file ("
              << logFilePath_.string() << ") passed as parameter" << std::endl;
  }
}
void ProblemGenerationFileLogger::DisplayMessage(const std::string& message) {
  logFile_ << Prefix() << message << std::endl;
  logFile_.flush();
}

void ProblemGenerationFileLogger::DisplayMessage(const std::string& message,
                                                 const LOGLEVEL logLevel) {
  logFile_ << LogLevelToStr(logLevel) << message << std::endl;
  logFile_.flush();
}

ProblemGenerationOstreamLogger::ProblemGenerationOstreamLogger(
    const LOGLEVEL logLevel, std::ostream& stream)
    : ProblemGenerationILogger(logLevel), stream_(stream) {
  if (stream_.fail()) {
    std::cerr
        << "ProblemGenerationOstreamLogger: Invalid stream passed as parameter"
        << std::endl;
  }
}
void ProblemGenerationOstreamLogger::DisplayMessage(
    const std::string& message) {
  stream_ << Prefix() << message << std::endl;
}
void ProblemGenerationOstreamLogger::DisplayMessage(const std::string& message,
                                                    const LOGLEVEL logLevel) {
  stream_ << LogLevelToStr(logLevel) << message << std::endl;
}

void ProblemGenerationLogger::DisplayMessage(const std::string& message) {
  for (auto& logger : loggers_) {
    // std::visit(logger.)
    logger->DisplayMessage(message);
  }
}
void ProblemGenerationLogger::DisplayMessage(const std::string& message,
                                             const LOGLEVEL logLevel) {
  for (auto& logger : loggers_) {
    logger->DisplayMessage(message, logLevel);
  }
}

ProblemGenerationLogger& operator<<(ProblemGenerationLogger& logger,
                                    const LOGLEVEL logLevel) {
  for (auto& subLogger : logger.loggers_) {
    subLogger->GetOstreamObject() << LogLevelToStr(logLevel);
  }
  return logger;
}

ProblemGenerationLogger& operator<<(ProblemGenerationLogger& logger,
                                    std::ostream& (*f)(std::ostream&)) {
  for (auto& subLogger : logger.loggers_) {
    subLogger->GetOstreamObject() << f;
  }
  return logger;
}
}  // namespace ProblemGenerationLog