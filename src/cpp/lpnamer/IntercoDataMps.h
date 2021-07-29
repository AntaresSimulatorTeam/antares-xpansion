#pragma once

#include "common_lpnamer.h"
#include "Candidate.h"

#define CANDIDATES_INI "candidates.ini"
#define STRUCTURE_FILE "structure.txt"
#define MPS_TXT "mps.txt"
#define STUDY_FILE "study.antares"



struct ProblemData
{
	ProblemData(const std::string& problem_mps, const std::string& variables_txt, const std::string& contraintes_txt);
	std::string _problem_mps;
	std::string _variables_txt;
	std::string _contraintes_txt;
};


/*!
 *  \struct Candidates
 *  \brief Candidates structure
 *
 */
struct Candidates : public std::vector<Candidate> {

	static std::vector<ProblemData> MPS_LIST;			/*!< vector of 3 strings in a vector corresponding to the name of a mps , variable and constraint file */


	Candidates() {

	}
	explicit Candidates(std::string  const & datas);

	void treat(std::string const& root, ProblemData const&, 
		std::map< std::pair<std::string, std::string>, int>& couplings, std::string const& solver_name);
	void treatloop(std::string const & root, std::map< std::pair<std::string, 
		std::string>, int>& couplings, std::string const& solver_name);
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
