#pragma once

#include "common.h"
#include <multisolver_interface/SolverAbstract.h>

class SensitivityPbModifier
{
public:
    explicit SensitivityPbModifier(double epsilon, double bestUb);
    ~SensitivityPbModifier() = default;

    std::shared_ptr<SolverAbstract> changeProblem(const std::map<int, std::string> &idToName, const std::shared_ptr<SolverAbstract> &lastMaster);

private:
    double _epsilon;
    double _best_ub;

    std::shared_ptr<SolverAbstract> change_objective(std::shared_ptr<SolverAbstract> &solverModel, int nbCandidates);
    std::shared_ptr<SolverAbstract> add_near_optimal_cost_constraint(std::shared_ptr<SolverAbstract> &solverModel, int nbCandidates);
};