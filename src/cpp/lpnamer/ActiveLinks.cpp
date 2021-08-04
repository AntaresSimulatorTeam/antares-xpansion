
#include "ActiveLinks.h"


void ActiveLinks::addCandidate(const CandidateData& data, const LinkProfile& already_install_link_profile, const LinkProfile& link_profile){

    ActiveLink link;
    _links.push_back(link);
}

ActiveLinksInitializer::ActiveLinksInitializer(){

};

ActiveLinks ActiveLinksInitializer::createActiveLinkFromCandidates(const std::vector<CandidateData>& candidateList,const std::map<std::string, LinkProfile>& profileMap){
    ActiveLinks result;

    for (const CandidateData& candidateData : candidateList) {
        Candidate candidate;
        candidate._data = candidateData;
        LinkProfile already_installed_link_profile = getProfile(profileMap, candidateData.already_installed_link_profile);
        LinkProfile link_profile = getProfile(profileMap, candidateData.link_profile);

        result.addCandidate(candidateData, already_installed_link_profile, link_profile);
    }

    return  result;
}

LinkProfile ActiveLinksInitializer::getProfile(const std::map<std::string, LinkProfile> &profileMap,
                                               const std::string &link_profile_name) const {
    LinkProfile link_profile;
    if(!link_profile_name.empty()){
        link_profile = profileMap.at(link_profile_name);
    }
    return link_profile;
}
