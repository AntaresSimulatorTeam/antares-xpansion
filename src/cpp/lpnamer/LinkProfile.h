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

    //! direct linkprofile values
    std::vector<double> _directLinkProfile;
    //! indirect linkprofile values if different from direct linkprofile
    std::vector<double> _indirectLinkProfile;

    std::string _name;

/*!
 *  \brief LinkProfile default constructor
 *
 */
    LinkProfile();

/*!
 *  \brief returns true if the direct link profile column is empty
 *
 */
    bool empty() const;

/*!
 *  \brief reads a linkprofile file into the LinkProfile structure
 *
 *  \note input file format verification is delegated to the AntaresXpansionLauncher python module
 */
    void read(std::string const & file_path);

/*!
 *  \brief returns the value of a link profile
 *
 *  \param i : period for which to get the linkprofile value (between 0 and 8759)
 *  \param is_direct : if true, return the direct linkprofile value else the indirect
 */
    double get(size_t i, bool is_direct) const;

    /*!
    *  \brief returns the value of a direct link profile
    *
    *  \param i : period for which to get the linkprofile value (between 0 and 8759)
    */
    double getDirectProfile(size_t i) const;

    /*!
    *  \brief returns the value of a direct link profile
    *
    *  \param i : period for which to get the linkprofile value (between 0 and 8759)
    */
    double getIndirectProfile(size_t i) const;
};


#endif //ANTARESXPANSION_LINKPROFILE_H
