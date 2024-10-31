#ifndef __TESTS_LOGGER_UTILS_h__
#define __TESTS_LOGGER_UTILS_h__

#include "antares-xpansion/lpnamer/helper/ProblemGenerationLogger.h"

std::shared_ptr<ProblemGenerationLog::ProblemGenerationLogger> emptyLogger();
std::shared_ptr<ProblemGenerationLog::ProblemGenerationLogger>
BuildLoggerWithParams(const LogUtils::LOGLEVEL& logLevel,
                      const std::filesystem::path& FilePath);
std::shared_ptr<ProblemGenerationLog::ProblemGenerationLogger>
BuildLoggerWithParams(const LogUtils::LOGLEVEL& logLevel,
                      const std::filesystem::path& FilePath,
                      std::ostream& stream);
std::shared_ptr<ProblemGenerationLog::ProblemGenerationLogger>
BuildLoggerWithParams(const LogUtils::LOGLEVEL& logLevel, std::ostream& stream);
#endif  //__TESTS_LOGGER_UTILS_h__
