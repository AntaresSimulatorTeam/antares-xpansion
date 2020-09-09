#include <algorithm>

#include "IntercoDataMps.h"
#include "INIReader.h"

#include "ortools_utils.h"

namespace
{
	std::string toLowercase(std::string const & inputString_p)
	{
		std::string result;
		std::transform(inputString_p.cbegin(), inputString_p.cend(), std::back_inserter(result),[](char const & c) {
				return std::tolower(c);
		});
		return result;
	}
}


std::vector<std::vector<std::string> > Candidates::MPS_LIST = {
};

std::vector<std::tuple<int, int, int> > Candidates::intercos_map = {
	//#include "interco.txt"
};

//std::vector<std::vector<std::string>> Candidates::candidates_map = {
//#include "candidates.txt"
//};

std::map<int, std::string> Candidates::id_name = std::map<int, std::string>();

std::map<std::tuple<std::string, std::string>, int> Candidates::or_ex_id = std::map<std::tuple<std::string, std::string>, int>();
std::set<std::string> Candidates::str_fields = std::set<std::string>({
	"name",
	"investment_type",
	"link",
	"linkor",
	"linkex",
	"link-profile",
	"already-installed-link-profile"
	});

std::set<std::string> Candidates::dbl_fields = std::set<std::string>({
	"annual-cost-per-mw",
	"max-investment",
	"unit-size",
	"max-units",
	"already-installed-capacity"
	});

std::vector<std::string> Candidates::area_names = {
};

double Candidate::profile(size_t i, std::string const & study_path, bool is_direct) {
	if (_profile.empty()) {
		if (has("link-profile")) {
			std::string const file_name = str("link-profile");
			std::string const profile_path(study_path + PATH_SEPARATOR + "user" + PATH_SEPARATOR + "expansion" + PATH_SEPARATOR + "capa" + PATH_SEPARATOR + file_name);
			_profile.read(profile_path);
		}
	}
	return _profile.get(i, is_direct);
}

double Candidate::already_installed_profile(size_t i, std::string const & study_path, bool is_direct) {
	if (_already_installed_profile.empty()) {
		if (has("already-installed-link-profile")) {
			std::string const file_name = str("already-installed-link-profile");
			std::string const profile_path(study_path + PATH_SEPARATOR + "user" + PATH_SEPARATOR + "expansion" + PATH_SEPARATOR + "capa" + PATH_SEPARATOR + file_name);
			_already_installed_profile.read(profile_path);
		}
	}
	return _already_installed_profile.get(i, is_direct);
}

double Candidate::obj()const {
	return _dbl.find("annual-cost-per-mw")->second;
}

double Candidate::lb() const {
	return 0;
}

double Candidate::ub() const {
	auto it(_dbl.find("max-investment"));
	if (it != _dbl.end()) {
		return it->second;
	}
	else {
		return unit_size()*max_unit();
	}
}

bool Candidate::has_already_installed_capacity() const {
	return _dbl.find("already-installed-capacity") != _dbl.end();
}

bool Candidate::has_already_installed_link_profile() const {
	return _str.find("already-installed-link-profile") != _str.end();
}
double Candidate::already_installed_capacity() const {
	auto it = _dbl.find("already-installed-capacity");
	if (it != _dbl.end()) {
		return it->second;
	}
	else {
		return 0;
	}
}


double Candidate::unit_size() const {
	return _dbl.find("unit-size")->second;
}
double Candidate::max_unit() const {
	return _dbl.find("max-units")->second;
}

bool Candidate::is_integer()const {
	return _dbl.find("unit-size") != _dbl.end();
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
 * \brief fill a map of interconnection candidate using the pair of string origin country and destination country as a key
 *
 * \param key_paysor_paysex map of candidate using the pair of string origin country and destination country as a key
 * \return void
 */
void Candidates::getListOfIntercoCandidates(map<std::pair<std::string, std::string>, Candidate *> & key_paysor_paysex) {
	for (std::pair<std::string const, Candidate> & pairNameCandidate : *this) {
		Candidate const & interco(pairNameCandidate.second);

		std::string const & paysor(interco.str("linkor"));
		std::string const & paysex(interco.str("linkex"));

		// Check if duplicate or reverse interco does already exist
		if (key_paysor_paysex.find({ paysor, paysex }) != key_paysor_paysex.end()) {
			std::cout << "duplicate interco : " << paysor << " - " << paysex << std::endl;
			std::exit(1);
		}
		if (key_paysor_paysex.find({ paysex, paysor }) != key_paysor_paysex.end()) {
			std::cout << "reverse interco already defined : " << paysex << " - " << paysor << std::endl;
			std::exit(1);
		}
		key_paysor_paysex[{paysor, paysex }] = &pairNameCandidate.second;
	}
}

/**
 * \brief Read constraints***.txt file and fill the list of constraints cstrList
 *
 * \param filePath String corresponding to the constraints***.txt file path
 * \param cstrList list of strings which contain constraints
 * \param sizeCstrList size_t correspond to the list size
 * \return void
 */
void Candidates::readCstrfiles(std::string const filePath,
							std::list<std::string> & cstrList,
							size_t & sizeCstrList)
{
	std::string line;
	std::ifstream file(filePath.c_str());
	if (!file.good()) {
		std::cout << "unable to open " << filePath << std::endl;
		std::exit(1);
	}
	while (std::getline(file, line)) {
		std::istringstream buffer(line);
		std::ostringstream name;
		std::string part;
		bool is_first(true);
		while (std::getline(buffer, part)) {
			if (!is_first) {
				name << part << "_";
			}
			else {
				is_first = false;
			}
		}
		cstrList.push_back(name.str());
		sizeCstrList += name.str().size() + 1;
	}
	file.close();
}

/**
 * \brief Read variable***.txt file and fill the list of variables varList
 *
 * \param filePath String corresponding to the variable***.txt file path
 * \param varList list of strings which contain variables
 * \param sizeVarList size_t correspond to the list size
 * \param interco_data map of NTC interconnections...
 * \param interco_id map of NTC interconnections IDs...
 * \param key_paysor_paysex map of interco candidates
 * \return void
 */
void Candidates::readVarfiles(std::string const filePath,
							std::list<std::string> & varList,
							size_t & sizeVarList,
							std::map<int, std::vector<int> > & interco_data,
							std::map<std::vector<int>, int> & interco_id,
							map<std::pair<std::string, std::string>, Candidate *> key_paysor_paysex)
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
			int pdt;
			std::string trash;
			buffer >> id;
			buffer >> trash;
			buffer >> pays;
			buffer >> interco;
			buffer >> pdt;

			std::string const & paysor(Candidates::area_names[std::get<1>(intercos_map[interco])]);
			std::string const & paysex(Candidates::area_names[std::get<2>(intercos_map[interco])]);
			//add one variable index for each interco ==> for each canddidate
			if (key_paysor_paysex.find({ paysor, paysex }) != key_paysor_paysex.end()) {
				interco_data[id] = { pays, interco, pdt };
				if (interco_id.find({ pays, interco }) == interco_id.end()) {
					//std::cout << "interco_id.size() is : " << std::setw(4) << interco_id.size() << std::endl;
					int new_id = interco_id.size();
					interco_id[{pays, interco }] = new_id;

					//std::cout << "interco_id.size() is : " << std::setw(4) << interco_id.size() << std::endl;
					//for (auto const& kvp : interco_id) {
					//	std::cout << std::setw(4) << kvp.first[0];
					//	std::cout << std::setw(4) << kvp.first[1];
					//	std::cout << std::setw(4) << kvp.second;
					//	std::cout << std::endl;
					//}

				}
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
											std::map<std::vector<int>, int> interco_id,
											std::map< std::pair<std::string, std::string>, int> & couplings,
											map<std::pair<std::string, std::string>, Candidate *> key_paysor_paysex,
											std::string study_path,
											std::string const lp_mps_name)
{
	// XPRSsetintcontrol(xpr, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_NO_OUTPUT);
	//XPRSsetintcontrol(xpr, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_FULL_OUTPUT);
	// XPRSsetcbmessage(xpr, optimizermsg, NULL);
	operations_research::MPSolver in_prblm("read_problem", ORTOOLS_LP_SOLVER_TYPE);
	ORTreadmps(in_prblm, mps_name);

	int ncols(in_prblm.NumVariables());
	int nrows(in_prblm.NumConstraints());

	// check if number of rows in the solver matrix is equal to the number of constraints
	if (nrows != cstr.size()) {
		std::cout << "WRONG NUMBER OF CSTR NAMES, solver = " << nrows << ", " << cstr.size() << " given" << std::endl;
	}

	// check if number of columns in the solver matrix is equal to the number of variables
	if (ncols != var.size()) {
		std::cout << "WRONG NUMBER OF VAR NAMES, solver = " << ncols << ", " << var.size() << " given" << std::endl;
	}

	int ninterco_pdt = interco_data.size();

	std::vector<double> lb;
	std::vector<double> ub;
	std::vector<char> coltype;
	ORTgetcolinfo(in_prblm, coltype, lb, ub, 0, ncols - 1);
	std::vector<double> posinf(ninterco_pdt, in_prblm.infinity());
	std::vector<double> neginf(ninterco_pdt, -in_prblm.infinity());
	std::vector<char> lb_char(ninterco_pdt, 'L');
	std::vector<char> ub_char(ninterco_pdt, 'U');
	std::vector<int> indexes;
	indexes.reserve(ninterco_pdt);
	for (auto const & id : interco_data) {
		indexes.push_back(id.first);
	}
	// remove bounds on interco
	ORTchgbounds(in_prblm, indexes, lb_char, neginf);
	ORTchgbounds(in_prblm, indexes, ub_char, posinf);

	std::vector<std::string> vnames(var.begin(), var.end());
	operations_research::MPSolver out_prblm("new_problem", in_prblm.ProblemType());
	// copy in_prblm with the changed bounds and rename its variables
	ORTcopyandrenamevars(out_prblm, in_prblm, vnames);

	size_t cnt_l = 0;
	for(auto outVar_l : out_prblm.variables())
	{
		int outVarIndex_l = outVar_l->index();
		int originalIndex_l = std::stoi(in_prblm.variables()[outVarIndex_l]->name().substr(1));
		if( originalIndex_l != outVarIndex_l)
			std::cout << "WARNING : Variables names in subproblems may not be representative."
					<< " expected index " <<  originalIndex_l
					<< " for variable " << outVar_l->name() << " but index " << outVarIndex_l << " retrieved\n";
		++cnt_l;
	}


	// create pMax variable
	int ninterco = interco_id.size();
	//std::cout << "ninterco : " << ninterco << std::endl;
	std::vector<double> obj_interco(ninterco, 0);
	std::vector<double> lb_interco(ninterco, -out_prblm.infinity());
	std::vector<double> ub_interco(ninterco,  out_prblm.infinity());
	std::vector<char> coltypes_interco(ninterco, 'C');
	std::vector<std::string> colnames_l;

	for (auto const & interco : interco_id) {
		std::stringstream buffer;
		int interco_i = interco.first[1];
		int pays_or = std::get<1>(intercos_map[interco_i]);
		int pays_ex = std::get<2>(intercos_map[interco_i]);
		int pays = interco.first[0];
		//buffer << "INVEST_INTERCO_" << interco_i;
		buffer << id_name.find(interco_i)->second;

		// @FIXME check that names and variables orders correspond
		colnames_l.push_back(buffer.str());

		couplings[{buffer.str(), mps_name}] = interco.second + ncols;
		//std::cout << "ncols " << ncols << std::endl;
		//std::cout << "interco.second " << interco.second << std::endl;
		//std::cout << "buffer " << buffer.str() << std::endl;
	}

	ORTaddcols(out_prblm, obj_interco, {}, {}, {}, lb_interco, ub_interco, coltypes_interco, colnames_l);

	std::vector<double> dmatval;
	std::vector<int> colind;
	std::vector<char> rowtype;
	std::vector<double> rhs;
	std::vector<int> rstart;
	// create plower and upper constraint
	for (auto const & pairIdvarntcIntercodata : interco_data) {
		int const i_interco_pmax(interco_id.find({ pairIdvarntcIntercodata.second[0], pairIdvarntcIntercodata.second[1] })->second);
		int const i_interco_p(pairIdvarntcIntercodata.first);

		size_t timestep = pairIdvarntcIntercodata.second[2];

		int id_paysor(std::get<1>(intercos_map[pairIdvarntcIntercodata.second[1]]));
		int id_paysex(std::get<2>(intercos_map[pairIdvarntcIntercodata.second[1]]));
		std::string const & paysor(Candidates::area_names[id_paysor]);
		std::string const & paysex(Candidates::area_names[id_paysex]);

		Candidate & candidate(*(key_paysor_paysex.find({ paysor, paysex })->second));
		// p[t] - alpha[t].pMax - alpha0[t].pMax0 <= 0
		double already_installed_capacity( candidate.already_installed_capacity());
		rstart.push_back(dmatval.size());
		rhs.push_back(already_installed_capacity*candidate.already_installed_profile(timestep, study_path, true));
		rowtype.push_back('L');
		colind.push_back(i_interco_p);
		dmatval.push_back(1);
		colind.push_back(ncols + i_interco_pmax);
		dmatval.push_back(-candidate.profile(timestep, study_path, true));
		// p[t] + alpha[t].pMax + beta0[t].pMax0 >= 0
		rstart.push_back(dmatval.size());
		rhs.push_back(-already_installed_capacity*candidate.already_installed_profile(timestep, study_path, false));
		rowtype.push_back('G');
		colind.push_back(i_interco_p);
		dmatval.push_back(1);
		colind.push_back(ncols + i_interco_pmax);
		dmatval.push_back(candidate.profile(timestep, study_path, false));
	}

	ORTaddrows(out_prblm, rowtype, rhs, {}, rstart, colind, dmatval);

	ORTwritemps(out_prblm, lp_mps_name );
	std::cout << "lp_name : " << lp_mps_name << " done" << std::endl;
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
	std::vector<std::string> const & mps,
	std::map< std::pair<std::string, std::string>, int> & couplings) {

	std::map<std::pair<std::string, std::string>, Candidate *> key_paysor_paysex;
	std::string const study_path = root + PATH_SEPARATOR + ".." + PATH_SEPARATOR + "..";

	getListOfIntercoCandidates(key_paysor_paysex);

	// get path of file problem***.mps, variable***.txt and constraints***.txt
	std::string const mps_name(root + PATH_SEPARATOR + mps[0]);
	std::string const var_name(root + PATH_SEPARATOR + mps[1]);
	std::string const cstr_name(root + PATH_SEPARATOR + mps[2]);

	// new mps file in the new lp directory
	std::string const lp_name = mps[0].substr(0, mps[0].size() - 4);
	std::string const lp_mps_name = root + PATH_SEPARATOR + "lp" + PATH_SEPARATOR + lp_name + ".mps";

	// List of variables
	std::list<std::string> var;
	size_t vsize(0);

	// list of constraints
	std::list<std::string> cstr;
	size_t csize(0);

	std::map<int, std::vector<int> > interco_data;
	std::map<std::vector<int>, int> interco_id;

	readCstrfiles(cstr_name, cstr, csize);
	readVarfiles(var_name, var, vsize, interco_data, interco_id, key_paysor_paysex);
	createMpsFileAndFillCouplings(mps_name, var, vsize, cstr, csize, interco_data, interco_id, couplings, key_paysor_paysex, study_path, lp_mps_name);
}


/**
 * \brief Execute the treat function for each mps elements in the candidates MPS_LIST
 *
 * \param root String corresponding to the path where are located input data
 * \param couplings map of pair of strings associated to an int. Determine the correspondence between optimizer variables and interconnection candidates
 * \return void
 */
void Candidates::treatloop(std::string const & root, std::map< std::pair<std::string, std::string>, int>& couplings) {
	int n_mps(0);
	for (auto const & mps : Candidates::MPS_LIST) {
		treat(root, mps, couplings);
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
		std::cout << "-------------------------------------------" << std::endl;
		for (auto const & str : Candidates::str_fields) {
			std::string val = reader.Get(sectionName, str, "NA");
			if ((val != "NA") && (val != "na")) {
				std::cout << sectionName << " : " << str << " = " << val << std::endl;
				if (str == "link") {
					size_t i = val.find(" - ");
					if (i != std::string::npos) {
						std::string s1 = toLowercase(val.substr(0, i));
						std::string s2 = toLowercase(val.substr(i + 3, val.size()));
						std::cout << s1 << " and " << s2 << std::endl;
						(*this)[sectionName]._str["linkor"] = s1;
						(*this)[sectionName]._str["linkex"] = s2;
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
					std::string candidateName = toLowercase(val);
					(*this)[sectionName]._str["name"] = candidateName;
				}
				else {
					(*this)[sectionName]._str[str] = val;
				}
			}
		}
		for (auto const & str : Candidates::dbl_fields) {
			std::string val = reader.Get(sectionName, str, "NA");
			if (val != "NA") {
				//std::cout <<"|||  "<< str << " is " << val << std::endl;
				std::stringstream buffer(val);
				double d_val(0);
				buffer >> d_val;
				(*this)[sectionName]._dbl[str] = d_val;
			}
		}

		auto it = or_ex_id.find({ (*this)[sectionName]._str["linkor"], (*this)[sectionName]._str["linkex"] });
		if (it == or_ex_id.end()) {
			std::cout << "cannot link candidate to interco id" << std::endl;
		}
		else {
			id_name[it->second] = toLowercase((*this)[sectionName]._str["name"]);
			std::cout << "index is " << it->second << " and name is " << id_name[it->second] << std::endl;
		}
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
