//

#include "antares-xpansion/lpnamer/model/LinkProfile.h"

constexpr int MAX_LINK_PROFILE_HOUR = NUMBER_OF_HOUR_PER_YEAR - 1;

double LinkProfile::getDirectProfile(size_t hour) const {
  if (hour > MAX_LINK_PROFILE_HOUR) {
    auto log_location = LOGLOCATION;
    auto err_msg = "Link profiles can be requested between point 0 and 8759.";
    (*logger_)(LogUtils::LOGLEVEL::FATAL) << LOGLOCATION << err_msg;
    throw InvalidHourForProfile(err_msg, log_location);
  }

  return direct_link_profile[hour];
}

double LinkProfile::getIndirectProfile(size_t hour) const {
  if (hour > MAX_LINK_PROFILE_HOUR) {
    auto log_location = LOGLOCATION;
    auto err_msg = "Link profiles can be requested between point 0 and 8759.";
    (*logger_)(LogUtils::LOGLEVEL::FATAL) << LOGLOCATION << err_msg;
    throw InvalidHourForProfile(err_msg, log_location);
  }

  return indirect_link_profile[hour];
}
