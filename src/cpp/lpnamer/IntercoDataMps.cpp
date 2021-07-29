#include <algorithm>

#include "IntercoDataMps.h"
#include "INIReader.h"

#include "solver_utils.h"
#include "helpers/StringUtils.h"

std::vector<ProblemData> Candidates::MPS_LIST = {
};

std::vector<std::tuple<int, int, int> > Candidates::intercos_map = {
};


std::map<int, std::string> Candidates::id_name = std::map<int, std::string>();

std::map<std::tuple<std::string, std::string>, int> Candidates::or_ex_id = std::map<std::tuple<std::string, std::string>, int>();


std::vector<std::string> Candidates::area_names = {
};

std::string Candidate::_capacitySubfolder = std::string(PATH_SEPARATOR) + "user" + PATH_SEPARATOR + "expansion" + PATH_SEPARATOR + "capa";

double Candidate::profile(size_t i, std::string const & study_path, bool is_direct) {
	if (_profile.empty()) {
        if(!_data.link_profile.empty()){
            std::string const file_name = _data.link_profile;
            std::string const profile_path(study_path + _capacitySubfolder + PATH_SEPARATOR + file_name);
            _profile.read(profile_path);

        }
	}
	return _profile.get(i, is_direct);
}

double Candidate::already_installed_profile(size_t i, std::string const & study_path, bool is_direct) {
	if (_already_installed_profile.empty()) {
	    if(!_data.already_installed_link_profile.empty()){
            std::string const file_name = _data.already_installed_link_profile;
            std::string const profile_path(study_path + _capacitySubfolder + PATH_SEPARATOR + file_name);
            _already_installed_profile.read(profile_path);
	    }
	}
	return _already_installed_profile.get(i, is_direct);
}

double Candidate::obj()const {
	return _data.annual_cost_per_mw;
}

double Candidate::lb() const {
	return 0;
}

double Candidate::ub() const {
	double val = _data.max_investment;
	if(val == 0.0) {
	    val = unit_size() * max_unit();
	}
	return val;
}

bool Candidate::has_already_installed_capacity() const {
	return _dbl.find("already-installed-capacity") != _dbl.end();
}

bool Candidate::has_already_installed_link_profile() const {
	return _str.find("already-installed-link-profile") != _str.end();
}
double Candidate::already_installed_capacity() const {
    return _data.already_installed_capacity;
}


double Candidate::unit_size() const {
	return _data.unit_size;
}
double Candidate::max_unit() const {
	return _data.max_units;
}

bool Candidate::is_integer()const {
	return _data.unit_size!=0.0;
}

/*!
 *  \brief Constructor
 *
 *  Construct the structure Candidates
 *
 *  \param ini_file path to the file candidate.ini
 */
Candidates::Candidates(std::string  const & ini_file) {
	getCandidatesFromFile(ini_file);
}

/**
 * \brief Read variable***.txt file and fill the list of variables varList
 *
 * \param filePath String corresponding to the variable***.txt file path
 * \param varList list of strings which contain variables
 * \param sizeVarList size_t correspond to the list size
 * \param interco_data map of NTC interconnections...
 * \param interco_id map of NTC interconnections IDs...
 * \return void
 */
void Candidates::readVarfiles(std::string const filePath,
							std::list<std::string> & varList,
							size_t & sizeVarList,
							std::map<int, std::vector<int> > & interco_data)
{
	std::string line;
	std::ifstream file(filePath.c_str());
	if (!file.good()) {
		std::cout << "unable to open " << filePath << std::endl;
		std::exit(1);
	}
	while (std::getline(file, line)) {
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
            if (contains(name.str(), "ValeurDeNTC")) {
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

			//TODO : adapt for multicandidate
			auto it = std::find_if(begin(), end(), [interco] (const Candidate& candidate){
                return candidate._data.link_id == interco;
			});

			if (it != end()){
                interco_data[id] = { interco, time_step};
			}
		}
		varList.push_back(name.str());
		sizeVarList += name.str().size() + 1;
	}
	file.close();
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
 * \param vsize correspond to the var list size
 * \param cstr list of constraints
 * \param csize correspond to the cstr list size
 * \param interco_data list of NTC interco
 * \param couplings map of pair of strings associated to an int. Determine the correspondence between optimizer variables and interconnection candidates
 * \return void
 */
void Candidates::createMpsFileAndFillCouplings(std::string const & mps_name,
											std::list<std::string> var,
											size_t vsize,
											std::list<std::string> cstr,
											size_t csize,
											std::map<int, std::vector<int> > interco_data,
											std::map< std::pair<std::string, std::string>, int> & couplings,
											std::string study_path,
											std::string const lp_mps_name,
											std::string const& solver_name)
{
	SolverFactory factory;
	SolverAbstract::Ptr in_prblm;
	in_prblm = factory.create_solver(solver_name);
	in_prblm->read_prob_mps(mps_name);

	int ncols = in_prblm->get_ncols();

	int ninterco_pdt = interco_data.size();
	std::cout << "ninterco_pdt " << ninterco_pdt << std::endl;

	std::vector<double> lb(ncols);
	std::vector<double> ub(ncols);
	std::vector<char> coltype(ncols);
    solver_getcolinfo(in_prblm, coltype, lb, ub, 0, ncols - 1);
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
	std::vector<std::string> vnames(var.begin(), var.end());
	SolverAbstract::Ptr out_prblm = factory.create_solver(in_prblm);

	// Xavier : Why do we need to copy the problem ?
	// We could just change the names in "in_prblm" and continuing modif it
	// This copy is just time consuming and useless for me

	// copy in_prblm with the changed bounds and rename its variables
    solver_copyandrenamevars(out_prblm, in_prblm, vnames, solver_name);
	size_t cnt_l = 0;
	// All the names are retrieved before the loop.
	// The vector might be huge. The names can be retrieved one by one from the solver in the loop
	// but it could be longer.
    std::vector<std::string> outVarNames = out_prblm->get_col_names(0, out_prblm->get_ncols() - 1);

	// create pMax variable
	int ninterco = size();
	std::vector<double> obj_interco(ninterco, 0);
	// Setting bounds to +-1e20
	std::vector<double> lb_interco(ninterco, -1e20);
	std::vector<double> ub_interco(ninterco,  1e20);
	std::vector<char> coltypes_interco(ninterco, 'C');
	std::vector<int> mstart_interco(ninterco, 0);
	std::vector<std::string> colnames_l;

	std::map<std::string ,int> interco_id;
	for (int i = 0 ; i < ninterco ; i++) {
	    const Candidate& candidate = at(i);

		colnames_l.push_back(candidate._data.name);

		couplings[{candidate._data.name, mps_name}] = i + ncols;
        interco_id[candidate._data.name] = i + ncols;
	}

    solver_addcols(out_prblm, obj_interco, mstart_interco, {}, {}, lb_interco, ub_interco, coltypes_interco, colnames_l);
	std::vector<double> dmatval;
	std::vector<int> colind;
	std::vector<char> rowtype;
	std::vector<double> rhs;
	std::vector<int> rstart;
	// create plower and upper constraint
	for (auto const & pairIdvarntcIntercodata : interco_data) {
	    int link_id = pairIdvarntcIntercodata.second[0];

        //TODO : adapt for multicandidate
        auto candidate = std::find_if(begin(), end(), [link_id] (const Candidate& candidate){
            return candidate._data.link_id == link_id;
        });

		int const i_interco_p(pairIdvarntcIntercodata.first);

		size_t timestep = pairIdvarntcIntercodata.second[1];


        //TO DO SFR
        // p[t] - alpha[t].(pMax_1 + pMax_2 + ...)  <= alpha0[t].pMax0
        double already_installed_capacity( candidate->already_installed_capacity());
        rstart.push_back(dmatval.size());
        rhs.push_back(already_installed_capacity*candidate->already_installed_profile(timestep, study_path, true));
        rowtype.push_back('L');
        colind.push_back(i_interco_p);
        dmatval.push_back(1);
        colind.push_back(interco_id[candidate->_data.name]);
        dmatval.push_back(-candidate->profile(timestep, study_path, true));
        // p[t] + alpha[t].pMax + beta0[t].pMax0 >= 0
        rstart.push_back(dmatval.size());
        rhs.push_back(-already_installed_capacity*candidate->already_installed_profile(timestep, study_path, false));
        rowtype.push_back('G');
        colind.push_back(i_interco_p);
        dmatval.push_back(1);
        colind.push_back(interco_id[candidate->_data.name]);
        dmatval.push_back(candidate->profile(timestep, study_path, false));
	}


	rstart.push_back(dmatval.size());

    solver_addrows(out_prblm, rowtype, rhs, {}, rstart, colind, dmatval);

	out_prblm->write_prob_mps(lp_mps_name);
	std::cout << "mps_name : " << lp_mps_name << " done" << std::endl;
}


/**
 * \brief That function create new optimization problems with new candidates
 *
 * \param root String corresponding to the path where are located input data
 * \param mps Strings vector with  the list of mps files
 * \param couplings map of pair of strings associated to an int. Determine the correspondence between optimizer variables and interconnection candidates
 * \return void
 */
void Candidates::treat(std::string const & root,
	ProblemData const & problemData,
	std::map< std::pair<std::string, std::string>, int> & couplings, std::string const& solver_name) {

	std::string const study_path = root + PATH_SEPARATOR + ".." + PATH_SEPARATOR + "..";


	// get path of file problem***.mps, variable***.txt and constraints***.txt
	std::string const mps_name(root + PATH_SEPARATOR + problemData._problem_mps);
	std::string const var_name(root + PATH_SEPARATOR + problemData._variables_txt);

	//TODO : remove constraint use
	std::string const cstr_name(root + PATH_SEPARATOR + problemData._contraintes_txt);

	// new mps file in the new lp directory
	std::string const lp_name = problemData._problem_mps.substr(0, problemData._problem_mps.size() - 4);
	std::string const lp_mps_name = root + PATH_SEPARATOR + "lp" + PATH_SEPARATOR + lp_name + ".mps";

	// List of variables
	std::list<std::string> var;
	size_t vsize(0);

	// list of constraints
	std::list<std::string> cstr;
	size_t csize(0);

	std::map<int, std::vector<int> > interco_data;

	readVarfiles(var_name, var, vsize, interco_data);
	createMpsFileAndFillCouplings(mps_name, var, vsize, cstr, csize, interco_data,
		couplings, study_path, lp_mps_name, solver_name);
}


/**
 * \brief Execute the treat function for each mps elements in the candidates MPS_LIST
 *
 * \param root String corresponding to the path where are located input data
 * \param couplings map of pair of strings associated to an int. Determine the correspondence between optimizer variables and interconnection candidates
 * \return void
 */
void Candidates::treatloop(std::string const & root, std::map< std::pair<std::string, std::string>,
	int>& couplings, std::string const& solver_name) {
	int n_mps(0);
	for (auto const & mps : Candidates::MPS_LIST) {
		treat(root, mps, couplings, solver_name);
		n_mps += 1;
	}
}


/**
 * \brief Fill the map of candidate from the file candidate.ini
 *
 * \param dataPath String corresponding to the "candidate.ini" file path
 * \return void
 */
void Candidates::getCandidatesFromFile(std::string  const & dataPath) {
	INIReader reader(dataPath.c_str());
	std::stringstream ss;
	std::set<std::string> sections = reader.Sections();
	for (auto const & sectionName : sections) {

	    Candidate candidate;

		for (auto const & str : Candidate::str_fields) {
			std::string val = reader.Get(sectionName, str, "NA");
			if ((val != "NA") && (val != "na")) {
				std::cout << sectionName << " : " << str << " = " << val << std::endl;
				if (str == "link") {
					size_t i = val.find(" - ");
					if (i != std::string::npos) {
						std::string s1 = StringUtils::ToLowercase(val.substr(0, i));
						std::string s2 = StringUtils::ToLowercase(val.substr(i + 3, val.size()));
						std::cout << s1 << " and " << s2 << std::endl;
                        candidate._str["linkor"] = s1;
                        candidate._str["linkex"] = s2;

						if(!this->checkArea(s1))
						{
							std::cout << "Unrecognized area " << s1
										<< " in section " << sectionName << " in " << dataPath << ".";
							std::exit(1);
						}
						if(!this->checkArea(s2))
						{
							std::cout << "Unrecognized area " << s2
										<< " in section " << sectionName << " in " << dataPath << ".";
							std::exit(1);
						}
					}
				}
				else if (str == "name")
				{
					std::string candidateName = StringUtils::ToLowercase(val);
                    candidate._str["name"] = candidateName;
				}
				else {
                    candidate._str[str] = val;
				}
			}
		}
		for (auto const & str : Candidate::dbl_fields) {
			std::string val = reader.Get(sectionName, str, "NA");
			if (val != "NA") {
				std::stringstream buffer(val);
				double d_val(0);
				buffer >> d_val;
                candidate._dbl[str] = d_val;
			}
		}

		auto it = or_ex_id.find({ candidate._str["linkor"], candidate._str["linkex"] });
		if (it == or_ex_id.end()) {
			std::cout << "cannot link candidate to interco id" << std::endl;
		}
		else {
			id_name[it->second] = StringUtils::ToLowercase(candidate._str["name"]);
			std::cout << "index is " << it->second << " and name is " << id_name[it->second] << std::endl;
		}

		//TODO : check if candidate is valid
        (*this).push_back(candidate);
	}
	std::cout << "-------------------------------------------" << std::endl;
}

/**
 * \brief checks if a given areaname is in the list of areas of candidates
 *
 * \param areaName_p String corresponding to the area name to verify
 * \return bool : True if the area exists in Candidates::area_names
 */
bool Candidates::checkArea(std::string const & areaName_p) const
{
	bool found_l = std::find(Candidates::area_names.cbegin(), Candidates::area_names.cend(), areaName_p) != Candidates::area_names.cend();
	return found_l;
}

ProblemData::ProblemData(const std::string& problem_mps, const std::string& variables_txt, const std::string& contraintes_txt):
	_problem_mps(problem_mps), _variables_txt(variables_txt), _contraintes_txt(contraintes_txt)
{
}
