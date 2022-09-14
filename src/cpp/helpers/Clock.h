
#pragma once

#include <ctime>
#include <string>

class Clock {
 public:
  Clock() = default;
  virtual ~Clock() = default;

  virtual std::time_t getTime();
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
std::string timeToStr(const std::time_t &time_p);
}