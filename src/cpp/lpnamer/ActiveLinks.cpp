
#include "ActiveLinks.h"


void ActiveLinks::addCandidate(const CandidateData& data, const LinkProfile& already_install_link_profile, const LinkProfile& link_profile){

    Candidate candidate;
    candidate._data = data;
    candidate._already_installed_profile = already_install_link_profile;
    candidate._profile = link_profile;

    if (_linksHashMap.find(data.link_id) == _linksHashMap.end())
    {
        ActiveLink link(data.link_id, data.linkor, data.linkex);
        link.setName(data.link);
        _linksHashMap[data.link_id] = link;
    }

    if (hasCandidate(candidate))
    {
        std::string message = "Candidate " + candidate._data.name + " duplication detected";
        throw std::runtime_error(message);
    }

    _linksHashMap[data.link_id].addCandidate(candidate);
}

bool ActiveLinks::hasCandidate(const Candidate& candidate) const
{
    const auto& it = _linksHashMap.find(candidate._data.link_id);
    bool linkExist = (it == _linksHashMap.end());
    return !linkExist && it->second.hasCandidate(candidate);
}

int ActiveLinks::size() const
{
    return _linksHashMap.size();
}

ActiveLinksInitializer::ActiveLinksInitializer(){

};

ActiveLinks ActiveLinksInitializer::createActiveLinkFromCandidates(const std::vector<CandidateData>& candidateList,const std::map<std::string, LinkProfile>& profileMap){
    ActiveLinks activeLinks;

    for (const CandidateData& candidateData : candidateList) {
        LinkProfile already_installed_link_profile = getProfile(profileMap, candidateData.already_installed_link_profile);
        LinkProfile link_profile = getProfile(profileMap, candidateData.link_profile);

        activeLinks.addCandidate(candidateData, already_installed_link_profile, link_profile);
    }

    return  activeLinks;
}

LinkProfile ActiveLinksInitializer::getProfile(const std::map<std::string, LinkProfile> &profileMap,
                                               const std::string &link_profile_name) const {
    LinkProfile link_profile;
    if(!link_profile_name.empty()){
        link_profile = profileMap.at(link_profile_name);
    }
    return link_profile;
}

ActiveLink::ActiveLink(int idInterco, const std::string& origin, const std::string& end):
    _idInterco(idInterco), _Origin(origin), _End(end)
{
}

void ActiveLink::setName(const std::string& nameInterco)
{
    _name = nameInterco;
}

void ActiveLink::setAlreadyInstalledLinkProfile(const LinkProfile& linkProfile)
{
    _already_installed_profile = linkProfile;
}

void ActiveLink::addCandidate(const Candidate& candidate)
{
    if (_profile.empty() && candidate.has_link_profile())
    {
        _profile = candidate._profile;
    }
    // TODO : partir du principe que le candidat n'a plus de already_installed_link_profile
    if (_already_installed_profile.empty() && candidate.has_already_installed_link_profile())
    {
        _already_installed_profile = candidate._already_installed_profile;
        _already_installed_profile_name = candidate._data.already_installed_link_profile;
    }

    if (!_already_installed_profile_name.empty() && _already_installed_profile_name != candidate._data.already_installed_link_profile)
    {
        std::string message = "Multiple already_installed_profile detected for link " + _name;
        throw std::runtime_error(message);
    }

    _candidates.push_back(candidate);
}

bool ActiveLink::hasCandidate(const Candidate& candidate) const
{
    return std::find_if(_candidates.begin(), _candidates.end(), [candidate](const Candidate& candidate2) -> bool {return candidate._data.name == candidate2._data.name; }) != _candidates.end();
}

double ActiveLink::direct_profile(size_t i) const
{
    return _profile.getDirectProfile(i);
}

double ActiveLink::indirect_profile(size_t i) const
{
    return _profile.getIndirectProfile(i);
}

double ActiveLink::already_installed_direct_profile(size_t i) const
{
    return _already_installed_profile.getDirectProfile(i);
}

double ActiveLink::already_installed_indirect_profile(size_t i) const
{
    return _already_installed_profile.getIndirectProfile(i);
}
