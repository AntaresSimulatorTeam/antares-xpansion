//

#include "Candidate.h"

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