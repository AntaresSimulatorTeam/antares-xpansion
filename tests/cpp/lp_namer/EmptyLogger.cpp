#include "EmptyLogger.h"

ProblemGenerationLog::ProblemGenerationLoggerSharedPointer emptyLogger() {
  return std::make_shared<ProblemGenerationLog::ProblemGenerationLogger>(
      ProblemGenerationLog::LOGLEVEL::NONE);
}