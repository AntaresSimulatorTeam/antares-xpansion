#pragma once
// #include "xprs_driver.h"
#include "common_lpnamer.h"


struct LinkProfile {
public:
	LinkProfile() {

	}
	bool empty() const{
		return _column1.empty();
	}
	void read(std::string const & file_path) {
		std::ifstream infile(file_path.c_str());
		if (!infile.good()) {
			std::cout << "unable to open : " << file_path << std::endl;
		}
		_column1.reserve(8760);
		double value;
		std::string line;
		for (size_t line_id(0); line_id < 8760; ++line_id) {
			if (std::getline(infile, line)) {
				std::stringstream buffer(line);
				buffer >> value;
				_column1.push_back(value);
				if (buffer >> value) {
					_column2.reserve(8760);
					_column2.push_back(value);
				}
			}
			else {
				std::cout << "error not enough line in link-profile " << file_path << std::endl;
			}
		}
		infile.close();
	}

	std::vector<double> _column1;
	std::vector<double> _column2;

	double get(size_t i, bool is_direct) {
		if (_column1.empty() && _column2.empty()) {
			return 1.0;
		}else if (!is_direct && !_column2.empty()) {
			return _column2[i];
		}
		else {
			return _column1[i];
		}		
	}
};

/*!
 *  \struct Candidate
 *  \brief Candidate structure
 *
 */
struct Candidate {
	std::map<std::string, std::string> _str; /*!<  map of string , string associated type of link (origin, destination) and the country */
	std::map<std::string, double> _dbl;

	LinkProfile _profile;
	LinkProfile _already_installed_profile;

	int _id;

	/**
	 * \fn dbl(std::string const & key)
	 * \brief Get the element of _dbl associated to the key "key" or 0.0 if the key doe not exist
	 *
	 * \param key String corresponding to the key
	 * \return string
	 */
	double dbl(std::string const & key)const {
		auto const it(_dbl.find(key));
		return it == _dbl.end() ? 0.0 : it->second;
	}

	/**
	 * \fn str(std::string const & key)
	 * \brief Get the element of _str associated to the key "key"
	 *
	 * \param key String corresponding to the key
	 * \return string
	 */
	std::string str(std::string const & key)const {
		auto const it(_str.find(key));
		return it->second;
	}

	/**
	 * \fn has(std::string const & key)
	 * \brief Check if the key "key" is present in _str
	 *
	 * \param key String corresponding to the key
	 * \return bool
	 */
	bool has(std::string const & key)const {
		auto const it(_str.find(key));
		return it != _str.end();
	}

	double profile(size_t i, std::string const & study_path, bool is_direct);
	double already_installed_profile(size_t i, std::string const & study_path, bool is_direct);

	double obj()const;
	double lb()const;
	double ub()const;

	double unit_size() const;
	double max_unit() const;

	bool is_integer()const;

	bool has_already_installed_capacity() const;
	double already_installed_capacity() const;

	bool has_already_installed_link_profile() const;
	
};


/*!
 *  \struct Candidates
 *  \brief Candidates structure
 *
 */
struct Candidates : public std::map<std::string, Candidate> {

	static std::vector<std::vector<std::string> > MPS_LIST;			/*!< vector of 3 strings in a vector corresponding to the name of a mps , variable and constraint file */
	static std::vector<std::tuple<int, int, int> >  intercos_map;	/*!< vector of 3 int in tuple which correspond to interconnections */

	static std::map<std::tuple<std::string, std::string>, int> or_ex_id; /*!< map of tuple < origin country, destination country> associated to the int id of the interconnection */

	static std::map<int, std::string> id_name; /*!< id interco --> name of candidate in candidates.ini */

	static std::set<std::string> str_fields;
	static std::set<std::string> dbl_fields;

	static std::vector<std::string> area_names;						/*!< vector of string corresponding to area */


	Candidates() {

	}
	Candidates(std::string  const & datas);

	void treat(std::string const & root, std::vector<std::string> const &, std::map< std::pair<std::string, std::string>, int>& couplings);
	void treatloop(std::string const & root, std::map< std::pair<std::string, std::string>, int>& couplings);
	void getCandidatesFromFile(std::string  const & dataPath);
	void getListOfIntercoCandidates(map<std::pair<std::string, std::string>, Candidate *> & key_paysor_paysex);
	void readCstrfiles(std::string const filePath, std::list<std::string> & list, size_t & sizeList);
	void readVarfiles(std::string const filePath,
			          std::list<std::string> & list,
					  size_t & sizeList,
					  std::map<int, std::vector<int> > & interco_data ,
					  std::map<std::vector<int>, int> & interco_id,
					  map<std::pair<std::string, std::string>, Candidate *> key_paysor_paysex);
	void createMpsFileAndFillCouplings(std::string const & mps_name,
									   std::list<std::string> var,
									   size_t vsize,
									   std::list<std::string> cstr,
									   size_t csize,
									   std::map<int, std::vector<int> > interco_data,
									   std::map<std::vector<int>, int> interco_id,
									   std::map< std::pair<std::string, std::string>, int> & couplings,
									   map<std::pair<std::string, std::string>, Candidate *>  key_paysor_paysex,
									   std::string study_path,
									   std::string const lp_mps_name);
	bool checkArea(std::string const & areaName_p) const;

};
