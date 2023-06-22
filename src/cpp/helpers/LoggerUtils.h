#pragma once
#include "Clock.h"
#include "LogUtils.h"

inline std::string PrefixMessage(const LogUtils::LOGLEVEL &log_level,
                                 const std::string &context) {
  return "[" + context + "][" + LogUtils::LogLevelToStr(log_level) + " " +
         clock_utils::timeToStr(std::time(nullptr)) + "] ";
}
