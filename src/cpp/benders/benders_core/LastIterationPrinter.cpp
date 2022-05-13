#include "LastIterationPrinter.h"

#include <time.h>

#include <iostream>
#include <sstream>

inline void localtime_platformu(const std::time_t &time_p,
                                struct tm &local_time) {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  localtime_s(&local_time, &time_p);
#else  // defined(__unix__) || (__APPLE__)
  localtime_r(&time_p, &local_time);
#endif
}
std::string timeToStru(const long int time_in_seconds) {
  std::time_t seconds(
      time_in_seconds);  // you have to convert your input_seconds into time_t
  std::tm *p = std::gmtime(&seconds);  // convert to broken down time
  std::stringstream ss;
  const auto days = p->tm_yday;
  const auto hours = p->tm_hour;
  const auto mins = p->tm_min;
  const auto secs = p->tm_sec;
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
    ss << seconds << " seconds ";
  }
  return ss.str();
}
LastIterationPrinter::LastIterationPrinter(const LogData &data) : _data(data) {}
void LastIterationPrinter::print() const {
  std::cout << "Restart Study..." << std::endl;

  std::cout << "Elapsed time: " << timeToStru(_data.benders_elapsed_time)
            << std::endl;
  std::cout << "Performed Iterations= " << _data.it << std::endl;
}
