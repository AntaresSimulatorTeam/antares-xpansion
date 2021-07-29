#pragma once

#include "common_lpnamer.h"
#include "Candidate.h"

#define CANDIDATES_INI "candidates.ini"
#define STRUCTURE_FILE "structure.txt"
#define MPS_TXT "mps.txt"
#define STUDY_FILE "study.antares"

struct ProblemData
{
	ProblemData(const std::string& problem_mps, const std::string& variables_txt);
	std::string _problem_mps;
	std::string _variables_txt;
};


/*!
 *  \struct Candidates
 *  \brief Candidates structure
 *
 */
class Candidates : public std::vector<Candidate> {

public:

	Candidates() {}
    void treatloop(std::string const & root, std::map< std::pair<std::string,
            std::string>, int>& couplings, std::string const& solver_name);

private:

    std::vector<ProblemData> readMPSList(std::string const & mps_filePath_p);

	void treat(std::string const& root, ProblemData const&, 
		std::map< std::pair<std::string, std::string>, int>& couplings, std::string const& solver_name);

	void readVarfiles(std::string const filePath,
			          std::list<std::string> & list,
					  std::map<int, std::vector<int> > & interco_data);
	void createMpsFileAndFillCouplings(std::string const & mps_name,
									   std::list<std::string> var,
									   std::map<int, std::vector<int> > interco_data,
									   std::map< std::pair<std::string, std::string>, int> & couplings,
									   std::string study_path,
									   std::string const lp_mps_name,
									   std::string const& solver_name);

};
