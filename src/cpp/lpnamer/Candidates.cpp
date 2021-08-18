#include <algorithm>

#include "Candidates.h"
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
 * \brief Read variable***.txt file and fill the list of variables varList
 *
 * \param filePath String corresponding to the variable***.txt file path
 * \param var_names list of strings which contain variables
 * \param interco_data map of NTC interconnections...
 * \param interco_id map of NTC interconnections IDs...
 * \return void
 */
void LinkProblemsGenerator::readVarfiles(std::string const filePath,
                              std::vector <std::string> &var_names,
                              std::map<colId, ColumnsToChange>& p_var_columns)
{
	std::string line;
	std::ifstream file(filePath.c_str());
	if (!file.good()) {
		std::cout << "unable to open " << filePath << std::endl;
		std::exit(1);
	}
	while (std::getline(file, line)) {
        std::string name = getVarNameFromLine(line);
        if (contains(name, "ValeurDeNTC")) {
			std::istringstream buffer(line);
            colId id;
			int pays;
			int link_id;
			int time_step;
			std::string trash;
			buffer >> id;
			buffer >> trash;
			buffer >> pays;
			buffer >> link_id;
			buffer >> time_step;

			auto it = std::find_if(_links.begin(), _links.end(), [link_id] (const ActiveLink& link){
                return link._idLink == link_id;
			});

			if (it != _links.end()){
                p_var_columns[link_id].push_back({id, time_step});
			}
		}
		var_names.push_back(name);
	}
	file.close();
}

std::string LinkProblemsGenerator::getVarNameFromLine(const std::string &line) const {
    std::ostringstream name;
    {
        std::istringstream buffer(line);
        std::string part;
        bool is_first(true);
        while (buffer >> part) {
            if (!is_first) {
                name << part << "_";
            }
            else {
                is_first = false;
            }
        }
    }
    return name.str();
}


/**
 * \brief This function redefine the optimization problem with new variables
 *
 * The function remove constraints on maximal capacity on interconnections candidates and create new variables which correspond to the capacity.
 * Write mps file in new lp directory and write couplings file.
 *
 *
 *
 * \param mps_name is the path to the current mps file
 * \param var list of variables
 * \param interco_data list of NTC interco
 * \param couplings map of pair of strings associated to an int. Determine the correspondence between optimizer variables and interconnection candidates
 * \return void
 */
void LinkProblemsGenerator::createMpsFileAndFillCouplings(std::string const & mps_name,
                                               const std::vector<std::string>& var_names,
                                               const std::map<colId , ColumnsToChange>& p_var_columns,
                                               std::map< std::pair<std::string, std::string>, int> & couplings,
                                               std::string const lp_mps_name,
                                               std::string const& solver_name)
{
	SolverFactory factory;
	SolverAbstract::Ptr in_prblm;
	in_prblm = factory.create_solver(solver_name);
	in_prblm->read_prob_mps(mps_name);

    solver_rename_vars(in_prblm, var_names, solver_name);

    auto problem_modifier = ProblemModifier();
    in_prblm = problem_modifier.changeProblem(std::move(in_prblm), _links, p_var_columns);
    std::map<std::string, unsigned int> col_id = problem_modifier.get_candidate_col_id();

    //TODO : update couplings creation
    Candidates candidates;
    for (const ActiveLink& link : _links){
        candidates.insert(candidates.end(),link.getCandidates().begin(), link.getCandidates().end());
    }

    for(const Candidate& candidate : candidates){
        couplings[{candidate._data.name, mps_name}] = col_id[candidate._data.name];
	}
	in_prblm->write_prob_mps(lp_mps_name);
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
	std::map< std::pair<std::string, std::string>, int> & couplings, std::string const& solver_name) {

	// get path of file problem***.mps, variable***.txt and constraints***.txt
	std::string const mps_name(root + PATH_SEPARATOR + problemData._problem_mps);
	std::string const var_name(root + PATH_SEPARATOR + problemData._variables_txt);

	// new mps file in the new lp directory
	std::string const lp_name = problemData._problem_mps.substr(0, problemData._problem_mps.size() - 4);
	std::string const lp_mps_name = root + PATH_SEPARATOR + "lp" + PATH_SEPARATOR + lp_name + ".mps";

	// List of variables
    std::vector<std::string> var_names;
    std::map<colId , ColumnsToChange> p_var_columns;

	readVarfiles(var_name, var_names,  p_var_columns);
	createMpsFileAndFillCouplings(mps_name, var_names, p_var_columns,
		couplings, lp_mps_name, solver_name);
}


/**
 * \brief Execute the treat function for each mps elements
 *
 * \param root String corresponding to the path where are located input data
 * \param couplings map of pair of strings associated to an int. Determine the correspondence between optimizer variables and interconnection candidates
 * \return void
 */
void LinkProblemsGenerator::treatloop(std::string const & root, std::map< std::pair<std::string, std::string>,
	int>& couplings, std::string const& solver_name) {

    std::string const mps_file_name			= root + PATH_SEPARATOR + MPS_TXT;

	for (auto const & mps : readMPSList(mps_file_name)) {
		treat(root, mps, couplings, solver_name);
	}
}

