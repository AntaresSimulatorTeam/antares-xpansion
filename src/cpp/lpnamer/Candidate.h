#ifndef ANTARESXPANSION_CANDIDATE_H
#define ANTARESXPANSION_CANDIDATE_H

#include "common_lpnamer.h"
#include "LinkProfile.h"

struct CandidateData {

    //TODO : value not used
    std::string investment_type;

    std::string link;
    int         link_id;
    std::string linkor;
    std::string linkex;
    std::string already_installed_link_profile;
    double already_installed_capacity;

    std::string name;
    std::string link_profile;
    double annual_cost_per_mw;
    double max_investment;
    double unit_size;
    double max_units;

    
};

/*!
 *  \class Candidate
 *  \brief Candidate class
 *
 */
class Candidate {

public:
    Candidate(){};
    Candidate(CandidateData data, const LinkProfile& profile);

    CandidateData _data;
    LinkProfile _profile;
    LinkProfile _already_installed_profile;

    double direct_profile(size_t timeStep) const;
    double indirect_profile(size_t timeStep) const;
    double already_installed_direct_profile(size_t timeStep) const;
    double already_installed_indirect_profile(size_t timeStep) const;

    double obj()const;
    double lb()const;
    double ub()const;

    double unit_size() const;
    double max_unit() const;

    bool is_integer()const;

    bool has_already_installed_capacity() const;
    double already_installed_capacity() const;

    bool has_already_installed_link_profile() const;
    bool has_link_profile() const;

    std::string _name;

};

#endif //ANTARESXPANSION_CANDIDATE_H
