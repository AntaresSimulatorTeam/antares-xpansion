#include "CandidatesInitializer.h"
#include "IntercoINIReader.h"
#include "LinkProfileReader.h"

#include "helpers/StringUtils.h"

LinkProfile getOrImportProfile(const std::string &capacitySubfolder, const LinkProfileReader &linkProfileReader,
                               std::map<std::string, LinkProfile> &mapLinkProfile, const std::string &profile_name) {
    LinkProfile profile;

    if (!profile_name.empty() && mapLinkProfile.find(profile_name) == mapLinkProfile.end()) {
        mapLinkProfile[profile_name] = linkProfileReader.ReadLinkProfile(capacitySubfolder + PATH_SEPARATOR + profile_name);
    }
    profile = mapLinkProfile[profile_name];
    return profile;
}

void initializedCandidates(std::string const & rootPath, Candidates & candidates) {
	// Get all mandatory path
	std::string const xpansion_user_dir = rootPath + PATH_SEPARATOR + ".." + PATH_SEPARATOR + ".." + PATH_SEPARATOR + "user" + PATH_SEPARATOR + "expansion";
	std::string const candidates_file_name = xpansion_user_dir + PATH_SEPARATOR + CANDIDATES_INI;
    std::string const capacity_folder = xpansion_user_dir + PATH_SEPARATOR + "capa";
	std::string const area_file_name	= rootPath + PATH_SEPARATOR + "area.txt";
	std::string const interco_file_name	= rootPath + PATH_SEPARATOR + "interco.txt";

    IntercoINIReader reader(interco_file_name,area_file_name);
    std::vector<CandidateData> candidateList = reader.readCandidateData(candidates_file_name);

    LinkProfileReader linkProfileReader;
    std::map<std::string, LinkProfile> mapLinkProfile;

    for (const CandidateData& candidateData : candidateList){
        Candidate candidate;
        candidate._data = candidateData;

        candidate._already_installed_profile = getOrImportProfile(capacity_folder, linkProfileReader, mapLinkProfile, candidateData.already_installed_link_profile);
        candidate._profile = getOrImportProfile(capacity_folder, linkProfileReader, mapLinkProfile, candidateData.link_profile);

        candidates.push_back(candidate);
    }
}
