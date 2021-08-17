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
                              std::map<int, std::vector<int> > & interco_data)
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
			int id;
			int pays;
			int interco;
			int time_step;
			std::string trash;
			buffer >> id;
			buffer >> trash;
			buffer >> pays;
			buffer >> interco;
			buffer >> time_step;

			auto it = std::find_if(_links.begin(), _links.end(), [interco] (const ActiveLink& link){
                return link._idLink == interco;
			});

			if (it != _links.end()){
                interco_data[id] = { interco, time_step};
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
                                               std::vector <std::string> var_names,
                                               std::map<int, std::vector<int> > interco_data,
                                               std::map< std::pair<std::string, std::string>, int> & couplings,
                                               std::string const lp_mps_name,
                                               std::string const& solver_name)
{
	SolverFactory factory;
	SolverAbstract::Ptr in_prblm;
	in_prblm = factory.create_solver(solver_name);
	in_prblm->read_prob_mps(mps_name);

	int ninterco_pdt = interco_data.size();

    solver_rename_vars(in_prblm, var_names, solver_name);

//#define new_method
#ifdef new_method
    std::map<colId , ColumnsToChange> p_var_columns = generate_p_var_columns(interco_data);
    auto problem_modifier = ProblemModifier();
    in_prblm = problem_modifier.changeProblem(std::move(in_prblm), _links, p_var_columns);
    std::map<std::string, unsigned int> col_id = problem_modifier.get_candidate_col_id();
#else

    // Setting bounds to +-1e20
    std::vector<double> posinf(ninterco_pdt, 1e20);
    std::vector<double> neginf(ninterco_pdt, -1e20);
    std::vector<char> lb_char(ninterco_pdt, 'L');
    std::vector<char> ub_char(ninterco_pdt, 'U');
    std::vector<int> indexes;
    indexes.reserve(ninterco_pdt);
    for (auto const & id : interco_data) {

        indexes.push_back(id.first);
    }
    // remove bounds on interco
    solver_chgbounds(in_prblm, indexes, lb_char, neginf);
    solver_chgbounds(in_prblm, indexes, ub_char, posinf);

    // All the names are retrieved before the loop.
	// The vector might be huge. The names can be retrieved one by one from the solver in the loop
	// but it could be longer.

    std::map<std::string, int> col_id = add_candidates_to_problem_and_get_candidates_col_id(in_prblm);
#endif

    //TODO : update couplings creation
    Candidates candidates;
    for (const ActiveLink& link : _links){
        candidates.insert(candidates.end(),link.getCandidates().begin(), link.getCandidates().end());
    }

    for(const Candidate& candidate : candidates){
        couplings[{candidate._data.name, mps_name}] = col_id[candidate._data.name];
	}
	std::vector<double> dmatval;
	std::vector<int> colind;
	std::vector<char> rowtype;
	std::vector<double> rhs;
	std::vector<int> rstart;
	// create plower and upper constraint
	for (auto const & pairIdvarntcIntercodata : interco_data) {
        int const i_interco_p(pairIdvarntcIntercodata.first);
        int const link_id = pairIdvarntcIntercodata.second[0];
        int const timestep = pairIdvarntcIntercodata.second[1];

        auto link = std::find_if(_links.begin(),_links.end(), [link_id](const ActiveLink& link){
            return link._idLink == link_id;
        });

        const std::vector<Candidate>& link_candidates = link->getCandidates();

        // p[t] - (alpha_1[t]*pMax1 + alpha_2[t]*pMax2 + ...)  <= alpha0[t].pMax0
        double already_installed_capacity( link->_already_installed_capacity);
        double direct_already_installed_profile_at_timestep = link->already_installed_direct_profile(timestep);
        rstart.push_back(dmatval.size());
        rhs.push_back(already_installed_capacity * direct_already_installed_profile_at_timestep);
        rowtype.push_back('L');
        colind.push_back(i_interco_p);
        dmatval.push_back(1);
        for (auto candidate:link_candidates){
            colind.push_back(col_id[candidate._data.name]);
            dmatval.push_back(-candidate.direct_profile(timestep));
        }
        // p[t] + alpha_1[t].pMax1 + alpha_2[t].pMax2 + ...  >=  - beta0[t].pMax0
        double indirect_already_installed_profile_at_timestep = link->already_installed_indirect_profile(timestep);
        rstart.push_back(dmatval.size());
        rhs.push_back(-already_installed_capacity*indirect_already_installed_profile_at_timestep);
        rowtype.push_back('G');
        colind.push_back(i_interco_p);
        dmatval.push_back(1);
        for (auto candidate:link_candidates){
            colind.push_back(col_id[candidate._data.name]);
            dmatval.push_back(candidate.indirect_profile(timestep));
        }
	}


	rstart.push_back(dmatval.size());

    solver_addrows(in_prblm, rowtype, rhs, {}, rstart, colind, dmatval);
	in_prblm->write_prob_mps(lp_mps_name);
}

std::map<colId, ColumnsToChange>
LinkProblemsGenerator::generate_p_var_columns(const std::map<int, std::vector<int>> &interco_data) const {
    std::map<unsigned int, ColumnsToChange> links_columns_to_change;
    for (auto const & pairIdvarntcIntercodata : interco_data) {
        colId const i_interco_p = pairIdvarntcIntercodata.first;
        int const link_id = pairIdvarntcIntercodata.second[0];
        int const timestep = pairIdvarntcIntercodata.second[1];
        links_columns_to_change[link_id].push_back({i_interco_p, timestep});
    }
    return links_columns_to_change;
}

std::map<std::string, int> LinkProblemsGenerator::add_candidates_to_problem_and_get_candidates_col_id(SolverAbstract::Ptr &out_prblm) {
    //TODO : update candidate col creation
    Candidates candidates;
    for (const ActiveLink& link : _links){
        candidates.insert(candidates.end(),link.getCandidates().begin(), link.getCandidates().end());
    }

    int n_candidates = candidates.size();
    int n_cols = out_prblm->get_ncols();
    std::vector<double> obj_interco(n_candidates, 0);
    // Setting bounds to +-1e20
    std::vector<double> lb_interco(n_candidates, -1e20);
    std::vector<double> ub_interco(n_candidates, 1e20);
    std::vector<char> coltypes_interco(n_candidates, 'C');
    std::vector<int> mstart_interco(n_candidates, 0);
    std::vector<std::string> candidates_colnames;

    std::map<std::string ,int> candidate_id;
    for (int i = 0 ; i < n_candidates ; i++) {
        const Candidate& candidate = candidates.at(i);
        candidates_colnames.push_back(candidate._data.name);
        candidate_id[candidate._data.name] = i + n_cols;
    }
    solver_addcols(out_prblm, obj_interco, mstart_interco, {}, {}, lb_interco, ub_interco, coltypes_interco, candidates_colnames);
    return candidate_id;
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
	std::map<int, std::vector<int> > interco_data;

	readVarfiles(var_name, var_names,  interco_data);
	createMpsFileAndFillCouplings(mps_name, var_names, interco_data,
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

