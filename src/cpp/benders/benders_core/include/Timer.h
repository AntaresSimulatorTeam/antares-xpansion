#pragma once

#include <chrono>

#include "common.h"
typedef std::chrono::time_point<std::chrono::system_clock> TimePoint;

class Timer {
 public:
  Timer();
  explicit Timer(const double begin_time);
  virtual ~Timer();

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

inline Timer::~Timer() {}
