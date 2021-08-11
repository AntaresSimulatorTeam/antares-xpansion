
#include "ActiveLinks.h"
#include "LinkProfileReader.h"
#include "Candidate.h"
#include <unordered_set>


void ActiveLinksBuilder::addCandidate(const CandidateData& candidate_data, const std::map<std::string, LinkProfile>& profile_map){

    int indexLink = getIndexOf(candidate_data.link_id);

    if (indexLink == -1)
    {
        ActiveLink link(candidate_data.link_id, candidate_data.link);
        link._already_installed_capacity = linkToAlreadyInstalledCapacity[candidate_data.link];
        _links.push_back(link);
        indexLink = _links.size() - 1;
    }

    _links[indexLink].addCandidate(candidate_data, profile_map);
}

ActiveLinksBuilder::ActiveLinksBuilder(const std::vector<CandidateData>& candidateList, const std::map<std::string, LinkProfile>& profile_map):
    _candidateDatas(candidateList), _profile_map(profile_map)
{
    checkCandidateNameDuplication();
    checkLinksValidity();
}

void ActiveLinksBuilder::checkLinksValidity()
{
    for (const auto& candidateData : _candidateDatas)
    {
        const std::string& link = candidateData.link;
        const std::string& link_profile = candidateData.link_profile;
        const std::string& already_installed_link_profile = candidateData.already_installed_link_profile;
        const double already_installed_link_capacity = candidateData.already_installed_capacity;

        launchExceptionIfNoLinkProfileAssociated(link_profile);
        launchExceptionIfNoLinkProfileAssociated(already_installed_link_profile);
        launchExceptionIfLinkHasAnotherAlreadyInstalledLinkProfile(link, already_installed_link_profile);
        launchExceptionIfLinkHasAnotherAlreadyInstalledCapacity(link, already_installed_link_capacity);
    }
}

void ActiveLinksBuilder::launchExceptionIfLinkHasAnotherAlreadyInstalledCapacity(const std::string& link, const double& already_installed_link_capacity)
{
    const auto& it_linkToAlreadyInstalledCapacity = linkToAlreadyInstalledCapacity.find(link);
    if (it_linkToAlreadyInstalledCapacity != linkToAlreadyInstalledCapacity.end() && it_linkToAlreadyInstalledCapacity->second != already_installed_link_capacity)
    {
        std::string message = "Multiple already installed capacity detected for link " + link;
        throw std::runtime_error(message);
    }
    else if (already_installed_link_capacity > 0)
    {
        linkToAlreadyInstalledCapacity[link] = already_installed_link_capacity;
    }
}

void ActiveLinksBuilder::launchExceptionIfNoLinkProfileAssociated(const std::string& profileName)
{
    if (!profileName.empty())
    {
        const auto& it_profile = _profile_map.find(profileName);

        if (it_profile == _profile_map.end())
        {
            std::string message = "There is no linkProfile associated with " + profileName;
            throw std::runtime_error(message);
        }
    }
    
}

void ActiveLinksBuilder::launchExceptionIfLinkHasAnotherAlreadyInstalledLinkProfile(const std::string& link_name, const std::string& already_installed_link_profile_name)
{
    const auto& it_linkToAlreadyInstalledProfileName = linkToAlreadyInstalledProfileName.find(link_name);
    if (it_linkToAlreadyInstalledProfileName == linkToAlreadyInstalledProfileName.end() && !already_installed_link_profile_name.empty())
    {
        linkToAlreadyInstalledProfileName[link_name] = already_installed_link_profile_name;
    }
    else if (it_linkToAlreadyInstalledProfileName != linkToAlreadyInstalledProfileName.end() && it_linkToAlreadyInstalledProfileName->second != already_installed_link_profile_name)
    {
        std::string message = "Multiple already_installed_profile detected for link " + link_name;
        throw std::runtime_error(message);
    }
}


void ActiveLinksBuilder::checkCandidateNameDuplication()
{
    std::unordered_set<std::string> setCandidatesNames;
    for (const auto& candidateData : _candidateDatas)
    {
        auto it_inserted = setCandidatesNames.insert(candidateData.name);
        if (!it_inserted.second)
        {
            std::string message = "Candidate " + candidateData.name + " duplication detected";
            throw std::runtime_error(message);
        }
    }
}

const std::vector<ActiveLink>& ActiveLinksBuilder::getLinks()
{
    if (_links.empty()){
        for (const auto& candidateData : _candidateDatas) {
            addCandidate(candidateData, _profile_map);
        }
    }

    return _links;
}

int ActiveLinksBuilder::getIndexOf(int link_id) const
{
    int index = -1;
    for (int i = 0; i <_links.size(); i++)
    {
        if (_links.at(i)._idInterco == link_id)
        {
            index = i;
            break;
        }
    }
    return index;
}

ActiveLink::ActiveLink(int idInterco, const std::string& linkName):
    _idInterco(idInterco), _name(linkName), _already_installed_capacity(1)
{
}

void ActiveLink::setAlreadyInstalledLinkProfile(const LinkProfile& linkProfile)
{
    _already_installed_profile = linkProfile;
}

void ActiveLink::addCandidate(const CandidateData& candidate_data, const std::map<std::string, LinkProfile>& profile_map)
{
    auto it = profile_map.find(candidate_data.link_profile);
    LinkProfile linkprofile;
    if (it != profile_map.end())
    {
        linkprofile = it->second;
    }
    Candidate candidate(candidate_data, linkprofile);
    auto it_already_installed_link_profile = profile_map.find(candidate_data.already_installed_link_profile);

    if (_already_installed_profile.empty() && it_already_installed_link_profile != profile_map.end())
    {
        _already_installed_profile = it_already_installed_link_profile->second;
    }

    _candidates.push_back(candidate);
}

const std::vector<Candidate>& ActiveLink::getCandidates() const
{
    return _candidates;
}

double ActiveLink::already_installed_direct_profile(size_t timeStep) const
{
    return _already_installed_profile.getDirectProfile(timeStep);
}

double ActiveLink::already_installed_indirect_profile(size_t timeStep) const
{
    return _already_installed_profile.getIndirectProfile(timeStep);
}
