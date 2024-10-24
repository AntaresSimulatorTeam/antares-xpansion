
#pragma once

#include <ctime>
#include <string>

class Clock {
 public:
  Clock() = default;
  virtual ~Clock() = default;

  virtual std::time_t getTime() {
    std::time_t beginTime = std::time(nullptr);
    return beginTime;
  }
};

inline void localtime_platform(const std::time_t &time_p,
                               struct tm &local_time) {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  localtime_s(&local_time, &time_p);
#else  // defined(__unix__) || (__APPLE__)
  localtime_r(&time_p, &local_time);
#endif
}
namespace clock_utils {
inline std::string timeToStr(const std::time_t &time_p) {
  struct tm local_time;
  localtime_platform(time_p, local_time);
  // localtime_r(&time_p, &local_time); // Compliant
  const char *FORMAT = "%d-%m-%Y %H:%M:%S";
  char buffer_l[100];
  strftime(buffer_l, sizeof(buffer_l), FORMAT, &local_time);
  std::string strTime_l(buffer_l);

  return strTime_l;
}
}