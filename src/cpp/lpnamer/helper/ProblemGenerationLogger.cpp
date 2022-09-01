#include "ProblemGenerationLogger.h"

namespace ProblemGenerationLog {

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

void ProblemGenerationLogger::DisplayMessage(const std::string& message) {
  for (auto& logger : loggers_) {
    logger->DisplayMessage(message);
  }
}

}  // namespace ProblemGenerationLog