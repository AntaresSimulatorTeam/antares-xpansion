#ifndef ANTARESXPANSION_LINKPROFILEREADER_H
#define ANTARESXPANSION_LINKPROFILEREADER_H

#include "LinkProfile.h"
#include "Candidate.h"

class LinkProfileReader {

public:

    LinkProfileReader(){}

    static LinkProfile ReadLinkProfile(const std::string& filename);
    static const std::map<std::string, LinkProfile> getLinkProfileMap(const std::string& capacity_folder, const std::vector<CandidateData>& candidateList);
private:
    static void importProfile(std::map<std::string, LinkProfile>& mapLinkProfile, const std::string& capacitySubfolder, const std::string& profile_name);

};


#endif //ANTARESXPANSION_LINKPROFILEREADER_H
