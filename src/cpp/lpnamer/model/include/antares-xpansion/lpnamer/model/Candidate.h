#ifndef ANTARESXPANSION_CANDIDATE_H
#define ANTARESXPANSION_CANDIDATE_H

#include "antares-xpansion/lpnamer/model/LinkProfile.h"
#include "antares-xpansion/lpnamer/helper/ProblemGenerationLogger.h"
#include "StringManip.h"

struct CandidateData {
  bool enable = true;

  std::string link_name;
  int link_id;
  std::string linkor;
  std::string linkex;
  std::string installed_direct_link_profile_name;
  std::string installed_indirect_link_profile_name;
  double already_installed_capacity = 1.0;

  std::string name;
  double annual_cost_per_mw = 0.0;
  double max_investment = 0.0;
  double unit_size = 0.0;
  double max_units = 0.0;
  std::string direct_link_profile;
  std::string indirect_link_profile;
};

/*!
 *  \class Candidate
 *  \brief Candidate class
 *
 */
class Candidate {
 public:
  Candidate() = default;
  Candidate(const CandidateData& data, std::vector<LinkProfile> profile);

  double directCapacityFactor(size_t timeStep) const;
  double indirectCapacityFactor(size_t timeStep) const;

  double directCapacityFactor(size_t chronicle_number, size_t timeStep) const;
  double indirectCapacityFactor(size_t chronicle_number, size_t timeStep) const;

  double obj() const;
  double lb() const;
  double ub() const;

  double unit_size() const;
  double max_unit() const;

  bool is_integer() const;

  std::string get_name() const;
  void set_name(const std::string& name);

  bool hasNullProfile(unsigned int chronicle,
                      const std::set<int>& time_steps) const;

  [[nodiscard]] unsigned long number_of_chronicles() const;

 private:
  std::vector<LinkProfile> _profile;
  std::string _name;
  double _annual_cost_per_mw;
  double _max_investment;
  double _unit_size;
  double _max_units;
};

#endif  // ANTARESXPANSION_CANDIDATE_H
