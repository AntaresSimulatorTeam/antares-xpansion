#include "CandidatesInitializer.h"


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
			std::vector<std::string> data;
			std::string str;
			while (buffer >> str) {
				data.push_back(str);
			}
			Candidates::MPS_LIST.push_back(data);
		}
	}
}

void initIntercoMap(std::string const & intercoFilepath_p)
{
    std::string line;

    Candidates::intercos_map.clear();
	std::ifstream interco_filestream(intercoFilepath_p.c_str());
	if (!interco_filestream.good()) {
		std::cout << "unable to open " << intercoFilepath_p << std::endl;
		std::exit(1);
	}
	while (std::getline(interco_filestream, line)) {
		std::stringstream buffer(line);
		if (!line.empty() && line.front() != '#') {
			int interco; /*!< Number of the interconnection */
			int pays_or; /*!< Number of the origin country */
			int pays_ex; /*!< Number of the destination country */

			buffer >> interco;
			buffer >> pays_or;
			buffer >> pays_ex;

			Candidates::intercos_map.push_back(std::make_tuple(interco, pays_or, pays_ex));
		}
	}
}


void initAreas(std::string const & areaFilepath_p)
{
    std::string line;

    Candidates::area_names.clear();
	std::ifstream area_filestream(areaFilepath_p.c_str());
	if (!area_filestream.good()) {
		std::cout << "unable to open " << areaFilepath_p << std::endl;
		std::exit(1);
	}
	while (std::getline(area_filestream, line)) {
		if (!line.empty() && line.front() != '#') {
			Candidates::area_names.push_back(line);
		}
	}
	for (auto const & kvp : Candidates::intercos_map) {
		std::string const & pays_or(Candidates::area_names[std::get<1>(kvp)]);
		std::string const & pays_ex(Candidates::area_names[std::get<2>(kvp)]);
		Candidates::or_ex_id[std::make_tuple(pays_or, pays_ex)] = std::get<0>(kvp);
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

	// Initialize the list of interconnexion
	initIntercoMap(interco_file_name);

	// Initialize the list of area
	initAreas(area_file_name);

    //read the candidates from file
	candidates.getCandidatesFromFile(candidates_file_name);
}
