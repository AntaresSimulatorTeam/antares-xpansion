#pragma once

#include <time.h>

#include <chrono>
#include <sstream>

typedef std::chrono::time_point<std::chrono::system_clock> TimePoint;

class Timer {
 public:
  Timer();
  explicit Timer(const double begin_time);
  virtual ~Timer() = default;

  double elapsed() const;
  void restart();

 private:
  TimePoint _start;
  double _begin_time = 0;
};

inline Timer::Timer() { restart(); }
inline Timer::Timer(const double begin_time) : _begin_time(begin_time) {
  // _start
  restart();
}

inline void Timer::restart() { _start = std::chrono::system_clock::now(); }

inline double Timer::elapsed() const {
  return std::chrono::duration<double>(std::chrono::system_clock::now() -
                                       _start)
             .count() +
         _begin_time;
}

inline std::string format_time_str(const long int time_in_seconds) {
  std::time_t seconds(
      time_in_seconds);  // you have to convert your input_seconds into time_t
  std::tm p;

  // convert to broken down time
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  gmtime_s(&p, &seconds);
#else  // defined(__unix__) || (__APPLE__)
  gmtime_r(&seconds, &p);
#endif
  std::stringstream ss;
  const auto days = p.tm_yday;
  const auto hours = p.tm_hour;
  const auto mins = p.tm_min;
  const auto secs = p.tm_sec;
  if (days > 0) {
    ss << days << " days ";
  }
  if (hours > 0) {
    ss << hours << " hours ";
  }
  if (mins > 0) {
    ss << mins << " minutes ";
  }
  if (secs > 0) {
    ss << secs << " seconds ";
  }
  return ss.str();
}