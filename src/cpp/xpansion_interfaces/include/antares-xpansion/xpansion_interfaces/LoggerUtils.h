#pragma once
#include "antares-xpansion/xpansion_interfaces/Clock.h"
#include "antares-xpansion/xpansion_interfaces/LogUtils.h"

inline std::string PrefixMessage(const LogUtils::LOGLEVEL &log_level,
                                 const std::string &context) {
  return "[" + context + "][" + LogUtils::LogLevelToStr(log_level) + " " +
         clock_utils::timeToStr(std::time(nullptr)) + "] ";
}
