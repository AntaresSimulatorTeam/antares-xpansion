
#pragma once

#include <ctime>

#include "Timer.h"

class Clock {
 public:
  Clock() = default;
  virtual ~Clock() = default;

  virtual std::time_t getTime();
};