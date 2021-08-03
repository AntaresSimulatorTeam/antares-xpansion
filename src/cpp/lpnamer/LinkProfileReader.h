#ifndef ANTARESXPANSION_LINKPROFILEREADER_H
#define ANTARESXPANSION_LINKPROFILEREADER_H

#include "LinkProfile.h"

class LinkProfileReader {

public:

    LinkProfileReader(){}

    static LinkProfile ReadLinkProfile(const std::string& filename);

};


#endif //ANTARESXPANSION_LINKPROFILEREADER_H
