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

/*!
 *  \brief reads a linkprofile file into the LinkProfile structure
 *
 *  \note input file format verification is delegated to the AntaresXpansionLauncher python module
 */
void LinkProfile::read(std::string const & file_path) {
    std::ifstream infile(file_path.c_str());
    if (!infile.good()) {
        std::cout << "unable to open : " << file_path << std::endl;
    }
    _directLinkProfile.reserve(8760);
    double value;
    std::string line;
    for (size_t line_id(0); line_id < 8760; ++line_id) {
        if (std::getline(infile, line)) {
            std::stringstream buffer(line);
            buffer >> value;
            _directLinkProfile.push_back(value);
            if (buffer >> value) {
                _indirectLinkProfile.reserve(8760);
                _indirectLinkProfile.push_back(value);
            }
        }
        else {
            std::cout << "error not enough line in link-profile " << file_path << std::endl;
        }
    }
    infile.close();
}

double LinkProfile::get(size_t i, bool is_direct) const {
    if (i > 8759)
    {
        std::cerr << "Link profiles can be requested between point 0 and 8759." << std::endl;
        std::exit(1);
    }

    if (_directLinkProfile.empty() && _indirectLinkProfile.empty()) {
        return 1.0;
    }else if (!is_direct && !_indirectLinkProfile.empty()) {
        return _indirectLinkProfile[i];
    }
    else {
        return _directLinkProfile[i];
    }
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