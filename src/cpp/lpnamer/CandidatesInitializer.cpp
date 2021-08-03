#include "CandidatesInitializer.h"
#include "CandidatesINIReader.h"
#include "LinkProfileReader.h"

#include "helpers/StringUtils.h"



CandidatesInitializer::CandidatesInitializer(LinkProfileReader& linkProfileReader, CandidatesINIReader& candidatesIniReader):
_linkProfileReader(linkProfileReader), _candidatesIniReader(candidatesIniReader){

}

Candidates CandidatesInitializer::initializedCandidates(std::string const & candidates_file_name, std::string const & capacity_folder) {
    Candidates  candidates;

    std::vector<CandidateData> candidateList = _candidatesIniReader.readCandidateData(candidates_file_name);

    for (const CandidateData& candidateData : candidateList){
        Candidate candidate;
        candidate._data = candidateData;

        candidate._already_installed_profile = getOrImportProfile(capacity_folder, candidateData.already_installed_link_profile);
        candidate._profile = getOrImportProfile(capacity_folder,  candidateData.link_profile);

        candidates.push_back(candidate);
    }

    return candidates;
}

LinkProfile CandidatesInitializer::getOrImportProfile(const std::string &capacity_folder, const std::string &profile_name) {
    LinkProfile profile;

    if (!profile_name.empty()){
        if(_mapLinkProfile.find(profile_name) == _mapLinkProfile.end()) {
            _mapLinkProfile[profile_name] = _linkProfileReader.ReadLinkProfile(capacity_folder + PATH_SEPARATOR + profile_name);
        }
        profile = _mapLinkProfile[profile_name];
    }
    return profile;
}
