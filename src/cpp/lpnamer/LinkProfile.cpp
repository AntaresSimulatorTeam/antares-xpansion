//

#include "LinkProfile.h"

LinkProfile::LinkProfile() {

}

/*!
 *  \brief returns true if the direct link profile column is empty
 *
 */
bool LinkProfile::empty() const{
    return _directLinkProfile.empty();
}

double LinkProfile::getDirectProfile(size_t i) const {
    if (i > 8759)
    {
        std::cerr << "Link profiles can be requested between point 0 and 8759." << std::endl;
        std::exit(1);
    }

    if (_directLinkProfile.empty()) {
        return 1.0;
    }else {
        return _directLinkProfile[i];
    }
}

double LinkProfile::getIndirectProfile(size_t i) const {
    if (i > 8759)
    {
        std::cerr << "Link profiles can be requested between point 0 and 8759." << std::endl;
        std::exit(1);
    }

    if (_directLinkProfile.empty() && _indirectLinkProfile.empty()) {
        return 1.0;
    }else if (!_indirectLinkProfile.empty()) {
        return _indirectLinkProfile[i];
    }else {
        return _directLinkProfile[i];
    }
}