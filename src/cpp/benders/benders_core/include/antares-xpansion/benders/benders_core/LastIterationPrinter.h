#ifndef __LASTITERATIONPRINTER_H__
#include "ILogger.h"

class LastIterationPrinter {
 public:
  LastIterationPrinter(Logger &logger, const LogData &best_iteration_data,
                       const LogData &last_iteration_data);
  void Print() const;

 private:
  Logger logger_;
  LogData best_iteration_data_;
  LogData last_iteration_data_;
};
#define __LASTITERATIONPRINTER_H__
#endif  // __LASTITERATIONPRINTER_H__