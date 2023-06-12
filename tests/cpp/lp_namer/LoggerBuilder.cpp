#include "LoggerBuilder.h"

ProblemGenerationLog::ProblemGenerationLoggerSharedPointer emptyLogger() {
  return std::make_shared<ProblemGenerationLog::ProblemGenerationLogger>(
      LogUtils::LOGLEVEL::NONE);
}
ProblemGenerationLog::ProblemGenerationILoggerSharedPointer FileLogger(
    const std::filesystem::path& FilePath) {
  return std::make_shared<ProblemGenerationLog::ProblemGenerationFileLogger>(
      FilePath);
}
ProblemGenerationLog::ProblemGenerationILoggerSharedPointer StreamLogger(
    std::ostream& stream) {
  return std::make_shared<ProblemGenerationLog::ProblemGenerationOstreamLogger>(
      stream);
}
ProblemGenerationLog::ProblemGenerationLoggerSharedPointer
BuildLoggerWithParams(const LogUtils::LOGLEVEL& loglevel,
                      const std::filesystem::path& FilePath,
                      std::ostream& stream) {
  auto fileLogger = FileLogger(FilePath);

  auto logger = BuildLoggerWithParams(loglevel, stream);
  logger->AddLogger(fileLogger);

  return logger;
}
ProblemGenerationLog::ProblemGenerationLoggerSharedPointer
BuildLoggerWithParams(const LogUtils::LOGLEVEL& loglevel,
                      const std::filesystem::path& FilePath) {
  auto fileLogger = FileLogger(FilePath);
  auto logger =
      std::make_shared<ProblemGenerationLog::ProblemGenerationLogger>(loglevel);
  logger->AddLogger(fileLogger);
  return logger;
}

ProblemGenerationLog::ProblemGenerationLoggerSharedPointer
BuildLoggerWithParams(const LogUtils::LOGLEVEL& loglevel,
                      std::ostream& stream) {
  auto StdOutLogger = StreamLogger(stream);
  auto logger =
      std::make_shared<ProblemGenerationLog::ProblemGenerationLogger>(loglevel);
  logger->AddLogger(StdOutLogger);
  return logger;
}
