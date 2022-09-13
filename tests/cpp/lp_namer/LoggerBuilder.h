#ifndef __TESTS_LOGGER_UTILS_h__
#define __TESTS_LOGGER_UTILS_h__

#include "ProblemGenerationLogger.h"

ProblemGenerationLog::ProblemGenerationLoggerSharedPointer emptyLogger();
ProblemGenerationLog::ProblemGenerationLoggerSharedPointer
BuildLoggerWithParams(const ProblemGenerationLog::LOGLEVEL& logLevel,
                      const std::filesystem::path& FilePath);
ProblemGenerationLog::ProblemGenerationLoggerSharedPointer
BuildLoggerWithParams(const ProblemGenerationLog::LOGLEVEL& logLevel,
                      const std::filesystem::path& FilePath,
                      std::ostream& stream);
ProblemGenerationLog::ProblemGenerationLoggerSharedPointer
BuildLoggerWithParams(const ProblemGenerationLog::LOGLEVEL& logLevel,
                      std::ostream& stream);
#endif  //__TESTS_LOGGER_UTILS_h__
