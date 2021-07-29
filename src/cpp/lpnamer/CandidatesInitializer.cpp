#include "CandidatesInitializer.h"
#include "IntercoINIReader.h"

#include "helpers/StringUtils.h"

void initMPSList(std::string const & mps_filePath_p)
{
    std::string line;

    Candidates::MPS_LIST.clear();
	std::ifstream mps_filestream(mps_filePath_p.c_str());
	if (!mps_filestream.good()) {
		std::cout << "unable to open " << mps_filePath_p << std::endl;
		std::exit(1);
	}
	while (std::getline(mps_filestream, line)) {
		std::stringstream buffer(line);
		if (!line.empty() && line.front() != '#') {
			
			std::string ProblemMps;
			std::string VariablesTxt;
			std::string ConstraintsTxt;

			buffer >> ProblemMps;
			buffer >> VariablesTxt;
			buffer >> ConstraintsTxt;

			ProblemData problemData(ProblemMps, VariablesTxt, ConstraintsTxt);
			
			Candidates::MPS_LIST.push_back(problemData);
		}
	}
}

void initializedCandidates(std::string const & rootPath, Candidates & candidates) {
	// Get all mandatory path
	std::string const candidates_file_name	= rootPath + PATH_SEPARATOR + ".." + PATH_SEPARATOR + ".." + PATH_SEPARATOR + "user" + PATH_SEPARATOR + "expansion" + PATH_SEPARATOR + CANDIDATES_INI;
	std::string const mps_file_name			= rootPath + PATH_SEPARATOR + MPS_TXT;
	std::string const area_file_name			= rootPath + PATH_SEPARATOR + "area.txt";
	std::string const interco_file_name		= rootPath + PATH_SEPARATOR + "interco.txt";

	// Initialize the list of MPS files
	initMPSList(mps_file_name);

    IntercoINIReader reader(interco_file_name,area_file_name);
    std::vector<CandidateData> candidateList = reader.readCandidateData(candidates_file_name);

    for (const CandidateData& candidateData : candidateList){
        Candidate candidate;
        candidate._data = candidateData;
        candidates.push_back(candidate);
    }
}
