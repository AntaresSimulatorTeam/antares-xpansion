#pragma once

#include "common.h"

#include <chrono>
typedef std::chrono::time_point<std::chrono::system_clock> TimePoint;

class Timer {
public:
	Timer();
	virtual ~Timer();

	double elapsed() const;
	void restart();
private:
	TimePoint _start;
};

inline Timer::Timer() {
	restart();

}

inline void Timer::restart() {
	_start = std::chrono::system_clock::now();

}

inline double Timer::elapsed() const {
	return std::chrono::duration < double
			> (std::chrono::system_clock::now() - _start).count();
}

inline Timer::~Timer() {
}
