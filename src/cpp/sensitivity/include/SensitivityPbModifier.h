#pragma once

#include "common.h"
#include <multisolver_interface/SolverAbstract.h>

class SensitivityPbModifier
{
public:
    SensitivityPbModifier() = default;
    explicit SensitivityPbModifier(double epsilon, double bestUb);
    ~SensitivityPbModifier() = default;

    SolverAbstract::Ptr changeProblem(const std::map<int, std::string> &idToName, const SolverAbstract::Ptr &lastMaster);

private:
    double _epsilon;
    double _best_ub;

    virtual SolverAbstract::Ptr change_objective(const SolverAbstract::Ptr &solverModel, int nbCandidates) const = 0;
    virtual SolverAbstract::Ptr add_near_optimal_cost_constraint(const SolverAbstract::Ptr &solverModel, int nbCandidates) const;
};