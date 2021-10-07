#include <algorithm>

#include "LinkProblemsGenerator.h"
#include "VariableFileReader.h"
#include "solver_utils.h"
#include "helpers/StringUtils.h"

ProblemData::ProblemData(const std::string& problem_mps, const std::string& variables_txt):
        _problem_mps(problem_mps), _variables_txt(variables_txt)
{
}

std::vector<ProblemData> LinkProblemsGenerator::readMPSList(std::string const & mps_filePath_p){
    std::string line;
    std::vector<ProblemData> result;
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

            ProblemData problemData(ProblemMps, VariablesTxt);

            result.push_back(problemData);
        }
    }

    return result;
}

/**
 * \brief That function create new optimization problems with new candidates
 *
 * \param root String corresponding to the path where are located input data
 * \param mps Strings vector with  the list of mps files
 * \param couplings map of pair of strings associated to an int. Determine the correspondence between optimizer variables and interconnection candidates
 * \return void
 */
void LinkProblemsGenerator::treat(std::string const & root,
	ProblemData const & problemData,
	std::map< std::pair<std::string, std::string>, int> & couplings) {

	// get path of file problem***.mps, variable***.txt and constraints***.txt
	std::string const mps_name(root + PATH_SEPARATOR + problemData._problem_mps);
	std::string const var_name(root + PATH_SEPARATOR + problemData._variables_txt);

	// new mps file in the new lp directory
	std::string const lp_name = problemData._problem_mps.substr(0, problemData._problem_mps.size() - 4);
	std::string const lp_mps_name = root + PATH_SEPARATOR + "lp" + PATH_SEPARATOR + lp_name + ".mps";

	// List of variables
    VariableFileReadNameConfiguration variable_name_config;
    variable_name_config.ntc_variable_name = "ValeurDeNTCOrigineVersExtremite";
    variable_name_config.cost_origin_variable_name = "CoutOrigineVersExtremiteDeLInterconnexion";
    variable_name_config.cost_extremite_variable_name = "CoutExtremiteVersOrigineDeLInterconnexion";

	VariableFileReader variableReader = VariableFileReader(var_name,_links,variable_name_config);

    std::vector<std::string> var_names = variableReader.getVariables();
    std::map<colId , ColumnsToChange> p_ntc_columns = variableReader.getNtcVarColumns();
    std::map<colId , ColumnsToChange> p_direct_cost_columns = variableReader.getDirectCostVarColumns();
    std::map<colId , ColumnsToChange> p_indirect_cost_columns = variableReader.getIndirectCostVarColumns();

    SolverFactory factory;
    SolverAbstract::Ptr in_prblm;
    in_prblm = factory.create_solver(_solver_name);
    in_prblm->read_prob_mps(mps_name);

    solver_rename_vars(in_prblm, var_names);

    auto problem_modifier = ProblemModifier();
    in_prblm = problem_modifier.changeProblem(std::move(in_prblm), _links, p_ntc_columns, p_direct_cost_columns,p_indirect_cost_columns);

    //couplings creation
    for (const ActiveLink& link : _links){
        for(const Candidate& candidate : link.getCandidates()){
            if (problem_modifier.has_candidate_col_id(candidate._name)){
                couplings[{candidate._name, mps_name}] = problem_modifier.get_candidate_col_id(candidate._name);
            }
        }
    }

    in_prblm->write_prob_mps(lp_mps_name);
}


/**
 * \brief Execute the treat function for each mps elements
 *
 * \param root String corresponding to the path where are located input data
 * \param couplings map of pair of strings associated to an int. Determine the correspondence between optimizer variables and interconnection candidates
 * \return void
 */
void LinkProblemsGenerator::treatloop(std::string const & root, std::map< std::pair<std::string, std::string>,
	int>& couplings) {

    std::string const mps_file_name			= root + PATH_SEPARATOR + MPS_TXT;

	for (auto const & mps : readMPSList(mps_file_name)) {
		treat(root, mps, couplings);
	}
}

