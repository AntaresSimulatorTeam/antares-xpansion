#ifndef __TESTS_LOGGER_UTILS_h__
#define __TESTS_LOGGER_UTILS_h__

#include "antares-xpansion/lpnamer/helper/ProblemGenerationLogger.h"

ProblemGenerationLog::ProblemGenerationLoggerSharedPointer emptyLogger();
ProblemGenerationLog::ProblemGenerationLoggerSharedPointer
BuildLoggerWithParams(const LogUtils::LOGLEVEL& logLevel,
                      const std::filesystem::path& FilePath);
ProblemGenerationLog::ProblemGenerationLoggerSharedPointer
BuildLoggerWithParams(const LogUtils::LOGLEVEL& logLevel,
                      const std::filesystem::path& FilePath,
                      std::ostream& stream);
ProblemGenerationLog::ProblemGenerationLoggerSharedPointer
BuildLoggerWithParams(const LogUtils::LOGLEVEL& logLevel, std::ostream& stream);
#endif  //__TESTS_LOGGER_UTILS_h__
