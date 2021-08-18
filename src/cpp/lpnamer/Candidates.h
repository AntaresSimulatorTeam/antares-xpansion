#pragma once

#include <memory>
#include <multisolver_interface/SolverAbstract.h>
#include "common_lpnamer.h"
#include "Candidate.h"
#include "ActiveLinks.h"


#define CANDIDATES_INI "candidates.ini"
#define STRUCTURE_FILE "structure.txt"
#define MPS_TXT "mps.txt"
#define STUDY_FILE "study.antares"
#include "ProblemModifier.h"

using Candidates = std::vector<Candidate>;

struct ProblemData
{
	ProblemData(const std::string& problem_mps, const std::string& variables_txt);
	std::string _problem_mps;
	std::string _variables_txt;
};

class LinkProblemsGenerator {

public:

	LinkProblemsGenerator(const std::vector<ActiveLink>& links):_links(links)
	{}

    void treatloop(std::string const & root, std::map< std::pair<std::string,
            std::string>, int>& couplings, std::string const& solver_name);

private:

    std::vector<ProblemData> readMPSList(std::string const & mps_filePath_p);

	void treat(std::string const& root, ProblemData const&, 
		std::map< std::pair<std::string, std::string>, int>& couplings, std::string const& solver_name);

	void readVarfiles(std::string const filePath,
                      std::vector <std::string> &var_names,
                      std::map<int, std::vector<int> > & interco_data);
	void createMpsFileAndFillCouplings(std::string const & mps_name,
                                       std::vector <std::string> var,
                                       std::map<int, std::vector<int> > interco_data,
                                       std::map< std::pair<std::string, std::string>, int> & couplings,
                                       std::string const lp_mps_name,
                                       std::string const& solver_name);

    std::string getVarNameFromLine(const std::string &line) const;

    std::map<colId, ColumnsToChange> generate_p_var_columns(const std::map<int, std::vector<int>> &interco_data) const;

    const std::vector<ActiveLink>& _links;
};
