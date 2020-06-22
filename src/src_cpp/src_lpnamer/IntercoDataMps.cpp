#include "IntercoDataMps.h"
#include "INIReader.h"


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
 * \fn void getListOfIntercoCandidates(map<std::pair<std::string, std::string>, Candidate *> & key_paysor_paysex)
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
			std::exit(0);
		}
		if (key_paysor_paysex.find({ paysex, paysor }) != key_paysor_paysex.end()) {
			std::cout << "reverse interco already defined : " << paysex << " - " << paysor << std::endl;
			std::exit(0);
		}
		key_paysor_paysex[{paysor, paysex }] = &pairNameCandidate.second;
	}
}

/**
 * \fn void readCstrfiles(std::string const filePath, std::list<std::string> & list, size_t & sizeList)
 * \brief Read constraints***.txt file and fill the list of constraints cstrList
 *
 * \param filePath String corresponding to the constraints***.txt file path
 * \param cstrList list of strings which contain constraints
 * \param sizeCstrList size_t correspond to the list size
 * \return void
 */
void Candidates::readCstrfiles(std::string const filePath,
	std::list<std::string> & cstrList,
	size_t & sizeCstrList) {
	std::string line;
	std::ifstream file(filePath.c_str());
	if (!file.good()) {
		std::cout << "unable to open " << filePath << std::endl;
		std::exit(0);
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
 * \fn void readVarfiles(std::string const filePath, std::list<std::string> & list, size_t & sizeList, std::map<int, std::vector<int> > & interco_data, std::map<std::vector<int>, int> & interco_id, map<std::pair<std::string, std::string>, Candidate *> key_paysor_paysex)
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
	map<std::pair<std::string, std::string>, Candidate *> key_paysor_paysex) {
	std::string line;
	std::ifstream file(filePath.c_str());
	if (!file.good()) {
		std::cout << "unable to open " << filePath << std::endl;
		std::exit(0);
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
 * \fn void createMpsFileAndFillCouplings()
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
void Candidates::createMpsFileAndFillCouplings(std::string const mps_name,
	std::list<std::string> var,
	size_t vsize,
	std::list<std::string> cstr,
	size_t csize,
	std::map<int, std::vector<int> > interco_data,
	std::map<std::vector<int>, int> interco_id,
	std::map< std::pair<std::string, std::string>, int> & couplings,
	map<std::pair<std::string, std::string>, Candidate *> key_paysor_paysex,
	std::string study_path,
	std::string const lp_mps_name) {
	XPRSprob xpr = NULL;
	XPRScreateprob(&xpr);
	XPRSsetintcontrol(xpr, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_NO_OUTPUT);
	//XPRSsetintcontrol(xpr, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_FULL_OUTPUT);
	XPRSsetcbmessage(xpr, optimizermsg, NULL);
	XPRSreadprob(xpr, mps_name.c_str(), "");

	int ncols;
	int nrows;
	XPRSgetintattrib(xpr, XPRS_COLS, &ncols);
	XPRSgetintattrib(xpr, XPRS_ROWS, &nrows);

	// check if number of rows in the XPRESS problem matrix is equal to the number of constraints
	if (nrows != cstr.size() + 1) {
		std::cout << "WRONG NUMBER OF CSTR NAMES, XPRESS = " << nrows << ", " << cstr.size() << " given" << std::endl;
	}

	// check if number of columns in the XPRESS problem matrix is equal to the number of variables
	if (ncols != var.size()) {
		std::cout << "WRONG NUMBER OF VAR NAMES, XPRESS = " << ncols << ", " << var.size() << " given" << std::endl;
	}

	int ninterco_pdt = interco_data.size();

	int status;
#ifdef __ADD_NAMES__
	std::vector<char> vnames(vsize, '\0');
	int iname(0);
	for (auto const & name : var) {
		for (int ichar(0); ichar < name.size(); ++ichar)
			vnames[iname + ichar] = name[ichar];
		iname += name.size() + 1;
	}
	status = XPRSaddnames(xpr, 2, vnames.data(), 0, ncols - 1);
	if (status) {
		std::cout << "XPRSaddnames error l." << __LINE__ << std::endl;
		std::exit(0);
	}
#endif

	std::vector<double> lb(ncols);
	std::vector<double> ub(ncols);
	XPRSgetlb(xpr, lb.data(), 0, ncols - 1);
	XPRSgetub(xpr, ub.data(), 0, ncols - 1);
	std::vector<double> posinf(ncols, XPRS_PLUSINFINITY);
	std::vector<double> neginf(ncols, XPRS_MINUSINFINITY);
	std::vector<char> lb_char(ncols, 'L');
	std::vector<char> ub_char(ncols, 'U');
	std::vector<int> indexes;
	indexes.reserve(ninterco_pdt);
	for (auto const & id : interco_data) {
		indexes.push_back(id.first);
	}
	// remove bounds on intero
	XPRSchgbounds(xpr, ninterco_pdt, indexes.data(), lb_char.data(), neginf.data());
	XPRSchgbounds(xpr, ninterco_pdt, indexes.data(), ub_char.data(), posinf.data());
	// create pMax variable
	int ninterco = interco_id.size();
	//std::cout << "ninterco : " << ninterco << std::endl;
	std::vector<int> mstart(ninterco + 1, 0);
	std::vector<double> obj_interco(ninterco, 0);
	std::vector<double> lb_interco(ninterco, +XPRS_MINUSINFINITY);
	std::vector<double> ub_interco(ninterco, +XPRS_PLUSINFINITY);
	status = XPRSaddcols(xpr, ninterco, 0, obj_interco.data(), mstart.data(), NULL, NULL, lb_interco.data(), ub_interco.data());
	if (status) {
		std::cout << "interco XPRSaddcols error" << std::endl;
		std::exit(0);
	}
	for (auto const & interco : interco_id) {
		std::stringstream buffer;
		int interco_i = interco.first[1];
		int pays_or = std::get<1>(intercos_map[interco_i]);
		int pays_ex = std::get<2>(intercos_map[interco_i]);
		int pays = interco.first[0];
		//buffer << "INVEST_INTERCO_" << interco_i;
		buffer << id_name.find(interco_i)->second;

#ifdef __ADD_NAMES__
		status = XPRSaddnames(xpr, 2, buffer.str().c_str(), ncols + interco.second, ncols + interco.second);
		if (status) {
			std::cout << "XPRSaddnames error l." << __LINE__ << std::endl;
			std::exit(0);
		}
#endif
		couplings[{buffer.str(), mps_name}] = interco.second + ncols;
		//std::cout << "ncols " << ncols << std::endl;
		//std::cout << "interco.second " << interco.second << std::endl;
		//std::cout << "buffer " << buffer.str() << std::endl;
	}

	std::vector<double> dmatval;
	std::vector<int> colind;
	std::vector<char> rowtype;
	std::vector<double> rhs;
	std::vector<int> rstart;
	// create plower and upper constraint
	for (auto const & kvp : interco_data) {
		int const i_interco_pmax(interco_id.find({ kvp.second[0], kvp.second[1] })->second);
		int const i_interco_p(kvp.first);

		std::string const & paysor(Candidates::area_names[std::get<1>(intercos_map[kvp.second[1]])]);
		std::string const & paysex(Candidates::area_names[std::get<2>(intercos_map[kvp.second[1]])]);

		Candidate & candidate(*(key_paysor_paysex.find({ paysor, paysex })->second));
		// p[t] - alpha.pMax - alpha0.pMax0 <= 0
		double already_installed_capacity( candidate.already_installed_capacity());
		rstart.push_back(dmatval.size());
		rhs.push_back(already_installed_capacity*candidate.already_installed_profile(kvp.second[2], study_path, true));
		rowtype.push_back('L');
		colind.push_back(i_interco_p);
		dmatval.push_back(1);
		colind.push_back(ncols + i_interco_pmax);
		dmatval.push_back(-candidate.profile(kvp.second[2], study_path, true));
		// p[t] + alpha.pMax + beta0.pMax0 >= 0
		rstart.push_back(dmatval.size());
		rhs.push_back(already_installed_capacity*candidate.already_installed_profile(kvp.second[2], study_path, false));
		rowtype.push_back('G');
		colind.push_back(i_interco_p);
		dmatval.push_back(1);
		colind.push_back(ncols + i_interco_pmax);
		dmatval.push_back(candidate.profile(kvp.second[2], study_path, false));
	}
	int n_row_interco(rowtype.size());
	int n_coeff_interco(dmatval.size());
	rstart.push_back(dmatval.size());
	status = XPRSaddrows(xpr, n_row_interco, n_coeff_interco, rowtype.data(), rhs.data(), NULL, rstart.data(), colind.data(), dmatval.data());
	if (status) {
		std::cout << "XPRSaddrows error l." << __LINE__ << std::endl;
		std::exit(0);
	}

	XPRSwriteprob(xpr, lp_mps_name.c_str(), "");
	XPRSdestroyprob(xpr);
	std::cout << "lp_name : " << lp_mps_name << " done" << std::endl;
}


/**
 * \fn void treat(root, mps, couplings)
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

	// new mpw file in the new lp directory
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
 * \fn void treatloop(std::string const & root, std::map< std::pair<std::string, std::string>, int>& couplings)
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
 * \fn getCandidatesFromFile(std::string  const & dataPath)
 * \brief Fill the map of candidate from the file candidate.ini
 *
 * \param dataPath String corresponding to the "candidate.ini" file path
 * \return void
 */
void Candidates::getCandidatesFromFile(std::string  const & dataPath) {
	INIReader reader(dataPath.c_str());
	std::stringstream ss;
	std::set<std::string> sections = reader.Sections();
	for (auto const & candidateName : sections) {
		std::cout << "-------------------------------------------" << std::endl;
		for (auto const & str : Candidates::str_fields) {
			std::string val = reader.Get(candidateName, str, "NA");
			if (val != "NA") {
				std::cout << candidateName << " : " << str << " = " << val << std::endl;
				if (str == "link") {
					size_t i = val.find(" - ");
					if (i != std::string::npos) {
						std::string s1 = val.substr(0, i);
						std::string s2 = val.substr(i + 3, val.size());
						std::cout << s1 << " and " << s2 << std::endl;
						(*this)[candidateName]._str["linkor"] = s1;
						(*this)[candidateName]._str["linkex"] = s2;
					}
				}
				else {
					(*this)[candidateName]._str[str] = val;
				}
			}
		}
		for (auto const & str : Candidates::dbl_fields) {
			std::string val = reader.Get(candidateName, str, "NA");
			if (val != "NA") {
				//std::cout <<"|||  "<< str << " is " << val << std::endl;
				std::stringstream buffer(val);
				double d_val(0);
				buffer >> d_val;
				(*this)[candidateName]._dbl[str] = d_val;
			}
		}

		auto it = or_ex_id.find({ (*this)[candidateName]._str["linkor"], (*this)[candidateName]._str["linkex"] });
		if (it == or_ex_id.end()) {
			std::cout << "cannot link candidate to interco id" << std::endl;
		}
		else {
			id_name[it->second] = (*this)[candidateName]._str["name"];
			std::cout << "index is " << it->second << " and name is " << id_name[it->second] << std::endl;
		}
	}
	std::cout << "-------------------------------------------" << std::endl;
}
