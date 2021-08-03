#include "Candidate.h"

double Candidate::direct_profile(size_t i) const{
    return _profile.getDirectProfile(i);
}

double Candidate::indirect_profile(size_t i) const{
    return _profile.getIndirectProfile(i);
}

double Candidate::already_installed_direct_profile(size_t i) const{
    return _already_installed_profile.getDirectProfile(i);
}

double Candidate::already_installed_indirect_profile(size_t i) const{
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