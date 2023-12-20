#pragma once

#include <boost/serialization/map.hpp>

#include "Worker.h"
#include "common.h"

struct SubProblemData {
  double subproblem_cost;
  Point var_name_and_subgradient;
  double subproblem_timer;
  int simplex_iter;
  int lpstatus;
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive &ar, const unsigned int version) {
    ar &subproblem_cost;
    ar &var_name_and_subgradient;
    ar &subproblem_timer;
    ar &simplex_iter;
    ar &lpstatus;
  }
};
using SubProblemDataMap = std::map<std::string, SubProblemData>;