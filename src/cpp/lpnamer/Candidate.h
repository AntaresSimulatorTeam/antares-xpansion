#ifndef ANTARESXPANSION_CANDIDATE_H
#define ANTARESXPANSION_CANDIDATE_H

#include "common_lpnamer.h"
#include "LinkProfile.h"

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

    //! folder containing the linkprofile files indicated in the candidates ini file
    static std::string _capacitySubfolder;

    CandidateData _data;
    LinkProfile _profile;
    LinkProfile _already_installed_profile;

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
