#pragma once

#include <boost/serialization/map.hpp>

#include "Worker.h"
#include "common.h"

namespace PlainData
{

struct SubProblemData
{
    double subproblem_cost{0.0};
    std::map<std::string, double> dual{};

    Point var_name_and_subgradient{};
    std::vector<double> criteria{};
    // no-supplied energy
    std::vector<double> patterns_values{};
    double single_subpb_costs_under_approx{0.0};
    double subproblem_timer{0.0};
    int simplex_iter{0};
    int lpstatus{0};
    friend class boost::serialization::access;
    double contribution_in_gap;

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar & subproblem_cost;
        ar & var_name_and_subgradient;
        ar & criteria;
        ar & patterns_values;
        ar & single_subpb_costs_under_approx;
        ar & subproblem_timer;
        ar & simplex_iter;
        ar & lpstatus;
        ar & contribution_in_gap;
    }
};
} // namespace PlainData

using SubProblemDataMap = std::map<std::string, PlainData::SubProblemData>;
