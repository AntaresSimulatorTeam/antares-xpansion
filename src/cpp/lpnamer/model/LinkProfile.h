#ifndef ANTARESXPANSION_LINKPROFILE_H
#define ANTARESXPANSION_LINKPROFILE_H

#include "common_lpnamer.h"

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
  LinkProfile() = default;

  /*!
   *  \brief returns true if the direct link profile column is empty
   *
   */
  bool empty() const;

  /*!
   *  \brief returns the value of a direct link profile
   *
   *  \param i : period for which to get the linkprofile value (between 0 and
   * 8759)
   */
  double getDirectProfile(size_t i) const;

  /*!
   *  \brief returns the value of a direct link profile
   *
   *  \param i : period for which to get the linkprofile value (between 0 and
   * 8759)
   */
  double getIndirectProfile(size_t i) const;

  //! direct linkprofile values
  std::vector<double> _directLinkProfile;
  //! indirect linkprofile values if different from direct linkprofile
  std::vector<double> _indirectLinkProfile;
};

#endif  // ANTARESXPANSION_LINKPROFILE_H
