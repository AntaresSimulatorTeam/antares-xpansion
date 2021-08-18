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

	LinkProblemsGenerator(const std::vector<ActiveLink>& links, const std::string& solver_name):
	_links(links),_solver_name(solver_name)
	{}

    void treatloop(std::string const & root, std::map< std::pair<std::string,
            std::string>, int>& couplings);

private:

    std::vector<ProblemData> readMPSList(std::string const & mps_filePath_p);

	void treat(std::string const& root, ProblemData const&, 
		std::map< std::pair<std::string, std::string>, int>& couplings);

	void readVarfiles(std::string const filePath,
                      std::vector <std::string> &var_names,
                      std::map<colId, ColumnsToChange>& p_var_columns);

    std::string getVarNameFromLine(const std::string &line) const;

    const std::vector<ActiveLink>& _links;
    std::string _solver_name;
};
