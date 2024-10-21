#include "antares-xpansion/helpers/Clock.h"

#include <ctime>
#include <string>

std::time_t Clock::getTime() {
  std::time_t beginTime = std::time(nullptr);
  return beginTime;
}

namespace clock_utils {
std::string timeToStr(const std::time_t &time_p) {
  struct tm local_time;
  localtime_platform(time_p, local_time);
  // localtime_r(&time_p, &local_time); // Compliant
  const char *FORMAT = "%d-%m-%Y %H:%M:%S";
  char buffer_l[100];
  strftime(buffer_l, sizeof(buffer_l), FORMAT, &local_time);
  std::string strTime_l(buffer_l);

  return strTime_l;
}
}  // namespace clock_utils