#ifndef __LASTITERATIONPRINTER_H__
#include "core/ILogger.h"

class LastIterationPrinter {
 public:
  LastIterationPrinter(Logger &logger, const LogData &best_iteration_data,
                       const LogData &last_iteration_data);
  void print() const;

 private:
  Logger _logger;
  LogData _best_iteration_data;
  LogData _last_iteration_data;
};
#define __LASTITERATIONPRINTER_H__
#endif  // __LASTITERATIONPRINTER_H__