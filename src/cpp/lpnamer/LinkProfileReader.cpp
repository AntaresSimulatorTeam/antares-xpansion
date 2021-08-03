//

#include "LinkProfileReader.h"

LinkProfile LinkProfileReader::ReadLinkProfile(const std::string& filename){

    std::ifstream infile(filename);
    if (!infile.good()) {
        throw std::runtime_error( "unable to open : " + filename);
    }
    LinkProfile result;
    result._directLinkProfile.reserve(8760);
    double value;
    std::string line;
    for (size_t line_id(0); line_id < 8760; ++line_id) {
        if (std::getline(infile, line)) {
            std::stringstream buffer(line);
            buffer >> value;
            result._directLinkProfile.push_back(value);
            if (buffer >> value) {
                result._indirectLinkProfile.reserve(8760);
                result._indirectLinkProfile.push_back(value);
            }
        }
        else {
            throw std::runtime_error( "error not enough line in link-profile " + filename);
        }
    }
    infile.close();

    return result;
}