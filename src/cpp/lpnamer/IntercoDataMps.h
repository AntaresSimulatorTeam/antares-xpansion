#pragma once
// #include "xprs_driver.h"
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
	static std::vector<std::tuple<int, int, int> >  intercos_map;	/*!< vector of 3 int in tuple which correspond to interconnections */

	static std::map<std::tuple<std::string, std::string>, int> or_ex_id; /*!< map of tuple < origin country, destination country> associated to the int id of the interconnection */

	static std::map<int, std::string> id_name; /*!< id interco --> name of candidate in candidates.ini */



	static std::vector<std::string> area_names;						/*!< vector of string corresponding to area */


	Candidates() {

	}
	explicit Candidates(std::string  const & datas);

	void treat(std::string const& root, ProblemData const&, 
		std::map< std::pair<std::string, std::string>, int>& couplings, std::string const& solver_name);
	void treatloop(std::string const & root, std::map< std::pair<std::string, 
		std::string>, int>& couplings, std::string const& solver_name);
	void getCandidatesFromFile(std::string  const & dataPath);
	void getListOfIntercoCandidates(std::map<std::pair<std::string, std::string>,
		std::list<Candidate *>> & key_paysor_paysex);
	void readCstrfiles(std::string const filePath, std::list<std::string> & list, size_t & sizeList);
	void readVarfiles(std::string const filePath,
			          std::list<std::string> & list,
					  size_t & sizeList,
					  std::map<int, std::vector<int> > & interco_data ,
					  std::map<std::vector<int>, int> & interco_id,
					  std::map<std::pair<std::string, std::string>, std::list<Candidate *>> key_paysor_paysex);
	void createMpsFileAndFillCouplings(std::string const & mps_name,
									   std::list<std::string> var,
									   size_t vsize,
									   std::list<std::string> cstr,
									   size_t csize,
									   std::map<int, std::vector<int> > interco_data,
									   std::map<std::vector<int>, int> interco_id,
									   std::map< std::pair<std::string, std::string>, int> & couplings,
									   std::map<std::pair<std::string, std::string>, std::list<Candidate *>>  key_paysor_paysex,
									   std::string study_path,
									   std::string const lp_mps_name,
									   std::string const& solver_name);
	bool checkArea(std::string const & areaName_p) const;

};
