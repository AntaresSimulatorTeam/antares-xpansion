#pragma once
#include <antares-xpansion/multisolver_interface/SolverAbstract.h>
#include <antares-xpansion/xpansion_interfaces/ILogger.h>
#include <unordered_map>

struct PresolveOptions;

class Presolve
{
public:
    SolverAbstract::Ptr init_solver(const PresolveOptions& options, Logger logger);
    std::unordered_map<int, int> get_candidates_presolve_map(SolverAbstract* solver,
                                                             std::vector<int>& candidatesId);
    void reduce_problems(SolverAbstract::Ptr& solver,
                         const PresolveOptions& options,
                         Logger logger);
};
