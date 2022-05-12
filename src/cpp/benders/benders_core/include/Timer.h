#pragma once

#include <chrono>

#include "common.h"
typedef std::chrono::time_point<std::chrono::system_clock> TimePoint;

class Timer {
 public:
  Timer();
  Timer(const double begin_time);
  virtual ~Timer();

  double elapsed() const;
  void restart();

 private:
  TimePoint _start;
};

inline Timer::Timer() { restart(); }
inline Timer::Timer(const double begin_time) {
  // _start
  _start = std::chrono::system_clock::now() +
           std::chrono::seconds((long int)begin_time);
}

inline void Timer::restart() { _start = std::chrono::system_clock::now(); }

inline double Timer::elapsed() const {
  return std::chrono::duration<double>(std::chrono::system_clock::now() -
                                       _start)
      .count();
}

inline Timer::~Timer() {}
