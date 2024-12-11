#pragma once

#include <boost/serialization/map.hpp>

#include "Worker.h"
#include "common.h"
namespace PlainData {

struct SubProblemData {
  double subproblem_cost;
  Point var_name_and_subgradient;
  std::vector<double> criteria;
  // no-supplied energy
  std::vector<double> patterns_values;
  double single_subpb_costs_under_approx;
  double subproblem_timer;
  int simplex_iter;
  int lpstatus;
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive &ar, const unsigned int version) {
    ar & subproblem_cost;
    ar & var_name_and_subgradient;
    ar & criteria;
    ar & single_subpb_costs_under_approx;
    ar & subproblem_timer;
    ar & simplex_iter;
    ar & lpstatus;
  }
};
}  // namespace PlainData

using SubProblemDataMap = std::map<std::string, PlainData::SubProblemData>;