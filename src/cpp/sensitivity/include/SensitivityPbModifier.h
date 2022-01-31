#pragma once

#include "common.h"
#include <multisolver_interface/SolverAbstract.h>

class SensitivityPbModifier
{
public:
    SensitivityPbModifier(){};
    explicit SensitivityPbModifier(double epsilon, double bestUb);
    ~SensitivityPbModifier() = default;

    SolverAbstract::Ptr changeProblem(const std::map<int, std::string> &idToName, const SolverAbstract::Ptr &lastMaster);

private:
    double _epsilon;
    double _best_ub;

    SolverAbstract::Ptr change_objective(SolverAbstract::Ptr &solverModel, int nbCandidates);
    SolverAbstract::Ptr add_near_optimal_cost_constraint(SolverAbstract::Ptr &solverModel, int nbCandidates);
};