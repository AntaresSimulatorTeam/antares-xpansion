#ifndef ANTARESXPANSION_CANDIDATE_H
#define ANTARESXPANSION_CANDIDATE_H

#include "common_lpnamer.h"

/*!
 *  \struct LinkProfile
 *  \brief LinkProfile structure
 *
 */

class LinkProfile {

public:

    //! direct linkprofile values
    std::vector<double> _column1;
    //! indirect linkprofile values if different from column1
    std::vector<double> _column2;

/*!
 *  \brief LinkProfile default constructor
 *
 */
    LinkProfile() {

    }

/*!
 *  \brief returns true if the direct link profile column is empty
 *
 */
    bool empty() const{
        return _column1.empty();
    }

/*!
 *  \brief reads a linkprofile file into the LinkProfile structure
 *
 *  \note input file format verification is delegated to the AntaresXpansionLauncher python module
 */
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

/*!
 *  \brief returns the value of a link profile
 *
 *  \param i : period for which to get the linkprofile value (between 0 and 8759)
 *  \param is_direct : if true, return the direct linkprofile value else the indirect
 */
    double get(size_t i, bool is_direct) const {
        if (i > 8759)
        {
            std::cerr << "Link profiles can be requested between point 0 and 8759." << std::endl;
            std::exit(1);
        }

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

struct CandidateData {

    std::string name;
    //TODO : value not used
    std::string investment_type;
    std::string link;
    int         link_id;

    std::string linkor;
    std::string linkex;
    std::string link_profile;
    std::string already_installed_link_profile;

    double annual_cost_per_mw;
    double max_investment;
    double unit_size;
    double max_units;
    double already_installed_capacity;
};

/*!
 *  \class Candidate
 *  \brief Candidate class
 *
 */
class Candidate {

public:

    static std::set<std::string> str_fields;
    static std::set<std::string> dbl_fields;

    //! folder containing the linkprofile files indicated in the candidates ini file
    static std::string _capacitySubfolder;

    std::map<std::string, std::string> _str; /*!<  map of string , string associated type of link (origin, destination) and the country */
    std::map<std::string, double> _dbl;

    LinkProfile _profile;
    LinkProfile _already_installed_profile;

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

#endif //ANTARESXPANSION_CANDIDATE_H
