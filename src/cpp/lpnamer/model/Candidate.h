#ifndef ANTARESXPANSION_CANDIDATE_H
#define ANTARESXPANSION_CANDIDATE_H

#include "LinkProfile.h"
#include "common_lpnamer.h"

struct CandidateData {

  bool enable = true;

  std::string link_name;
  int link_id;
  std::string linkor;
  std::string linkex;
  std::string installed_link_profile_name;
  double already_installed_capacity = 1.0;

  std::string name;
  std::string link_profile;
  double annual_cost_per_mw = 0.0;
  double max_investment = 0.0;
  double unit_size = 0.0;
  double max_units = 0.0;
};

/*!
 *  \class Candidate
 *  \brief Candidate class
 *
 */
class Candidate {

public:
  Candidate() = default;
  Candidate(const CandidateData &data, const LinkProfile &profile);

  double direct_profile(size_t timeStep) const;
  double indirect_profile(size_t timeStep) const;

  double obj() const;
  double lb() const;
  double ub() const;

  double unit_size() const;
  double max_unit() const;

  bool is_integer() const;

  std::string get_name() const;
  void set_name(const std::string &name);

private:
  LinkProfile _profile;
  std::string _name;
  double _annual_cost_per_mw;
  double _max_investment;
  double _unit_size;
  double _max_units;
};

#endif // ANTARESXPANSION_CANDIDATE_H
