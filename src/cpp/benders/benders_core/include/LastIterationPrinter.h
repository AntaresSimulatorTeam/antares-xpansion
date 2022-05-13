#ifndef __LASTITERATIONPRINTER_H__
#include "core/ILogger.h"

class LastIterationPrinter {
 public:
  LastIterationPrinter(const LogData &data);
  void print() const;

 private:
  LogData _data;
};
#define __LASTITERATIONPRINTER_H__
#endif  // __LASTITERATIONPRINTER_H__