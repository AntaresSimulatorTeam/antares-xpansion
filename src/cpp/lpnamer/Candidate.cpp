#include "Candidate.h"

Candidate::Candidate(CandidateData data, const std::map<std::string, LinkProfile>& profile_map)
{
    _data = data;
    _name = _data.name;
    auto it = profile_map.find(data.link_profile);
    if (it != profile_map.end())
    {
        _profile = it->second;
    }
}

double Candidate::direct_profile(size_t i) const{
    return _profile.getDirectProfile(i);
}

double Candidate::indirect_profile(size_t i) const{
    return _profile.getIndirectProfile(i);
}

double Candidate::already_installed_direct_profile(size_t i) const
{
    return _already_installed_profile.getDirectProfile(i);
}

double Candidate::already_installed_indirect_profile(size_t i) const
{
    return _already_installed_profile.getIndirectProfile(i);
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
    return _data.already_installed_capacity != 0.0;
}

bool Candidate::has_already_installed_link_profile() const {
    return !_data.already_installed_link_profile.empty();
}
bool Candidate::has_link_profile() const
{
    return !_data.link_profile.empty();
}
const std::string& Candidate::getName()
{
    return _name;
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