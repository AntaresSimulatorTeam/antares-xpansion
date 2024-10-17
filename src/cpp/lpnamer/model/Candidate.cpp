#include "antares-xpansion/lpnamer/model/Candidate.h"

#include <algorithm>
#include <utility>

Candidate::Candidate(const CandidateData& data,
                     std::vector<LinkProfile> profile)
    : _profile(std::move(profile)),
      _name(data.name),
      _annual_cost_per_mw(data.annual_cost_per_mw),
      _max_investment(data.max_investment),
      _unit_size(data.unit_size),
      _max_units(data.max_units) {}

double Candidate::directCapacityFactor(size_t timeStep) const {
  return _profile.at(0).getDirectProfile(timeStep);
}

double Candidate::indirectCapacityFactor(size_t timeStep) const {
  return _profile.at(0).getIndirectProfile(timeStep);
}

double Candidate::directCapacityFactor(size_t chronicle_number,
                                       size_t timeStep) const {
  /* When there is no scenario builder output from antares. The chronicle map
   * contains only zeros When there is an output the map contains values ranging
   * from 1 to <number of chronicles> The first case happen when there is only
   * one profile, so we need to return it The second case happen when you have
   * multiple profiles, in this case we need to project values from [1,N] to
   * [0,N-1] to return the proper profiles
   */
  if (chronicle_number == 0) {
    return _profile.at(0).getDirectProfile(timeStep);
  }
  if (chronicle_number > _profile.size()) chronicle_number = 1;
  // 1-based chronicle in 0 based vector
  return _profile.at(chronicle_number - 1).getDirectProfile(timeStep);
}

double Candidate::indirectCapacityFactor(size_t chronicle_number,
                                         size_t timeStep) const {
  if (chronicle_number == 0) {
    return _profile.at(0).getIndirectProfile(timeStep);
  }
  if (chronicle_number > _profile.size()) chronicle_number = 1;
  // 1-based chronicle in 0 based vector
  return _profile.at(chronicle_number - 1).getIndirectProfile(timeStep);
}

double Candidate::obj() const { return _annual_cost_per_mw; }

double Candidate::lb() const { return 0; }

double Candidate::ub() const {
  double val = _max_investment;
  if (val == 0.0) {
    val = unit_size() * max_unit();
  }
  return val;
}

double Candidate::unit_size() const { return _unit_size; }
double Candidate::max_unit() const { return _max_units; }

bool Candidate::is_integer() const { return _unit_size != 0.0; }

std::string Candidate::get_name() const { return _name; }

void Candidate::set_name(const std::string& name) { _name = name; }

bool Candidate::hasNullProfile(unsigned int chronicle,
                               const std::set<int>& time_steps) const {
  return std::all_of(
      time_steps.begin(), time_steps.end(), [this, chronicle](auto time_step) {
        return directCapacityFactor(chronicle, time_step) == 0.0 &&
               indirectCapacityFactor(chronicle, time_step) == 0.0;
      });
}
unsigned long Candidate::number_of_chronicles() const {
  return _profile.size();
}
