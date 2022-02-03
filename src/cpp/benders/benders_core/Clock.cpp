#include "Clock.h"
#include <ctime>
#include <string>

std::time_t Clock::getTime() {
  std::time_t beginTime = std::time(nullptr);
  return beginTime;
}
