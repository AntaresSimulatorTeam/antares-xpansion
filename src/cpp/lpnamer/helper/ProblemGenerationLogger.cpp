#include "ProblemGenerationLogger.h"
#include "Clock.h"

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

void ProblemGenerationLogger::DisplayMessage(const std::string& message)const {
  for (const auto& logger : loggers_) {
    logger->DisplayMessage(message);
  }
}
void ProblemGenerationLogger::DisplayMessage(const std::string& message,
                                             const LOGLEVEL logLevel)const {
  for (const auto& logger : loggers_) {
    logger->DisplayMessage(LogLevelToStr(logLevel));
    logger->DisplayMessage(message);
  }
}
std::string ProblemGenerationLogger::PrefixMessage(const LOGLEVEL& logLevel) const {
  return LogLevelToStr(logLevel) + clock_utils::timeToStr(std::time(nullptr))+  ": ";
}
ProblemGenerationLogger& ProblemGenerationLogger::operator<<(
    const LOGLEVEL logLevel) {
  for (const auto& subLogger : loggers_) {
    subLogger->GetOstreamObject() << PrefixMessage(logLevel);
  }
  return *this;
}

ProblemGenerationLogger& ProblemGenerationLogger::operator<<(
    std::ostream& (*f)(std::ostream&)) {
  for (const auto& subLogger : loggers_) {
    subLogger->GetOstreamObject() << f;
  }
  return *this;
}

}  // namespace ProblemGenerationLog