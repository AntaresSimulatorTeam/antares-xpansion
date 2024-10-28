#ifndef ANTARESXPANSION_LINKPROFILE_H
#define ANTARESXPANSION_LINKPROFILE_H

constexpr int NUMBER_OF_HOUR_PER_YEAR = 8760;
#include <array>
#include <exception>

#include "antares-xpansion/xpansion_interfaces/LogUtils.h"
#include "antares-xpansion/lpnamer/helper/ProblemGenerationLogger.h"
#include "antares-xpansion/xpansion_interfaces/StringManip.h"

/*!
 *  \struct LinkProfile
 *  \brief LinkProfile structure
 *
 */

class LinkProfile {
 public:
  /*!
   *  \brief LinkProfile default constructor
   *
   */
  explicit LinkProfile(
      ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger)
      : logger_(logger) {}

  /*!
   *  \brief returns the value of a direct link profile
   *
   *  \param hour : period for which to get the linkprofile value (between 0 and
   * 8759)
   */
  double getDirectProfile(size_t hour) const;

  /*!
   *  \brief returns the value of a direct link profile
   *
   *  \param hour : period for which to get the linkprofile value (between 0 and
   * 8759)
   */
  double getIndirectProfile(size_t hour) const;

  //! direct linkprofile values
  // ADR
  std::vector<double> direct_link_profile =
      std::vector<double>(NUMBER_OF_HOUR_PER_YEAR, 1);
  //! indirect linkprofile values if different from direct linkprofile
  std::vector<double> indirect_link_profile =
      std::vector<double>(NUMBER_OF_HOUR_PER_YEAR, 1);

  bool operator==(const LinkProfile& rhs) const {
    return direct_link_profile == rhs.direct_link_profile &&
           indirect_link_profile == rhs.indirect_link_profile;
  }

  class InvalidHourForProfile
      : public LogUtils::XpansionError<std::invalid_argument> {
    using LogUtils::XpansionError<std::invalid_argument>::XpansionError;
  };

 private:
  ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger_;
};

#endif  // ANTARESXPANSION_LINKPROFILE_H
