
#include "ActiveLinks.h"
#include "LinkProfileReader.h"
#include "Candidate.h"
#include <unordered_set>


void ActiveLinks::addCandidate(const CandidateData& candidate_data, const std::map<std::string, LinkProfile>& profile_map){

    int indexLink = getIndexOf(candidate_data.link_id);

    if (indexLink == -1)
    {
        ActiveLink link(candidate_data);
        _links.push_back(link);
        indexLink = _links.size() - 1;
    }

    _links[indexLink].addCandidate(candidate_data, profile_map);
}

void ActiveLinks::addCandidates(const std::vector<CandidateData>& candidateList, const std::map<std::string, LinkProfile>& profile_map)
{
    std::unordered_set<std::string> setCandidatesNames;
    for (const auto& candidateData : candidateList) {
        auto it_inserted = setCandidatesNames.insert(candidateData.name);
        if (!it_inserted.second)
        {
            std::string message = "Candidate " + candidateData.name + " duplication detected";
            throw std::runtime_error(message);
        }
        addCandidate(candidateData, profile_map);
    }
}

int ActiveLinks::getIndexOf(int link_id) const
{
    int index = -1;
    for (int i = 0; i <_links.size(); i++)
    {
        if (_links.at(i).getId() == link_id)
        {
            index = i;
            break;
        }
    }
    return index;
}

bool ActiveLinks::hasCandidate(const CandidateData& candidate_data) const
{
    bool hasCandidate = false;
    for (int i = 0; i < _links.size() && !hasCandidate; i++)
    {
        hasCandidate = _links.at(i).hasCandidate(candidate_data);
    }
    
    return hasCandidate;
}

int ActiveLinks::size() const
{
    return _links.size();
}

ActiveLink::ActiveLink(const CandidateData& candidate_data):
    _idInterco(candidate_data.link_id), _origin(candidate_data.linkor), _end(candidate_data.linkex),
    _name(candidate_data.link)
{
    
}

void ActiveLink::setAlreadyInstalledLinkProfile(const LinkProfile& linkProfile)
{
    _already_installed_profile = linkProfile;
}

void ActiveLink::addCandidate(const CandidateData& candidate_data, const std::map<std::string, LinkProfile>& profile_map)
{
    Candidate candidate(candidate_data, profile_map);
    auto it_already_installed_link_profile = profile_map.find(candidate_data.already_installed_link_profile);

    if (_already_installed_profile.empty() && it_already_installed_link_profile != profile_map.end())
    {
        _already_installed_profile = it_already_installed_link_profile->second;
        _already_installed_profile_name = it_already_installed_link_profile->first;
    }

    if (!_already_installed_profile_name.empty() && _already_installed_profile_name != it_already_installed_link_profile->first)
    {
        std::string message = "Multiple already_installed_profile detected for link " + _name;
        throw std::runtime_error(message);
    }

    _candidates.push_back(candidate);
}

bool ActiveLink::hasCandidate(const CandidateData& candidate_data) const
{
    for(const auto & cand: _candidates){
        if(cand._data.name == candidate_data.name){
            return true;
        }
    }
    return false;
}

int ActiveLink::getId() const
{
    return _idInterco;
}

double ActiveLink::already_installed_direct_profile(size_t timeStep) const
{
    return _already_installed_profile.getDirectProfile(timeStep);
}

double ActiveLink::already_installed_indirect_profile(size_t timeStep) const
{
    return _already_installed_profile.getIndirectProfile(timeStep);
}
