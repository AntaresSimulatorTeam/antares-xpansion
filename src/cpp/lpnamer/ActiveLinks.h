
#ifndef ANTARESXPANSION_ACTIVELINKS_H
#define ANTARESXPANSION_ACTIVELINKS_H

#include <Candidate.h>

class ActiveLink {

public:

    ActiveLink(){};
};

class ActiveLinks {

public:

    ActiveLinks(){};

    const std::vector<ActiveLink>& getLink() const {return _links;}

    void addCandidate(const CandidateData& data, const LinkProfile& already_install_link_profile, const LinkProfile& link_profile);

private:

    std::vector<ActiveLink> _links;
};

class ActiveLinksInitializer {

public:
    ActiveLinksInitializer();

    ActiveLinks createActiveLinkFromCandidates(const std::vector<CandidateData>& candidateList,const std::map<std::string, LinkProfile>& profileMap);

    LinkProfile
    getProfile(const std::map<std::string, LinkProfile> &profileMap, const std::string &link_profile_name) const;
};


#endif //ANTARESXPANSION_ACTIVELINKS_H
