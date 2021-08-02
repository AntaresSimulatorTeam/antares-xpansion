#include "CandidatesInitializer.h"
#include "IntercoINIReader.h"

#include "helpers/StringUtils.h"

void initializedCandidates(std::string const & rootPath, Candidates & candidates) {
	// Get all mandatory path
	std::string const candidates_file_name	= rootPath + PATH_SEPARATOR + ".." + PATH_SEPARATOR + ".." + PATH_SEPARATOR + "user" + PATH_SEPARATOR + "expansion" + PATH_SEPARATOR + CANDIDATES_INI;
	std::string const area_file_name			= rootPath + PATH_SEPARATOR + "area.txt";
	std::string const interco_file_name		= rootPath + PATH_SEPARATOR + "interco.txt";

    IntercoINIReader reader(interco_file_name,area_file_name);
    std::vector<CandidateData> candidateList = reader.readCandidateData(candidates_file_name);

    for (const CandidateData& candidateData : candidateList){
        Candidate candidate;
        candidate._data = candidateData;
        candidates.push_back(candidate);
    }
}
