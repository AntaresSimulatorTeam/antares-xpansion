#include "ProblemGenerationLogger.h"

static ProblemGenerationLog::ProblemGenerationLoggerSharedPointer
emptyLogger() {
  return std::make_shared<ProblemGenerationLog::ProblemGenerationLogger>(
      ProblemGenerationLog::LOGLEVEL::NONE);
}