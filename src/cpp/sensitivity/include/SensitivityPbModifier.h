#pragma once

#include "common.h"
#include <multisolver_interface/SolverAbstract.h>

class SensitivityPbModifier
{
public:
    explicit SensitivityPbModifier(double epsilon, double bestUb, std::map<int, std::string>& idToName, std::shared_ptr<SolverAbstract>& solverModel);
    ~SensitivityPbModifier() = default;

    std::shared_ptr<SolverAbstract> getProblem();
    void changeProblem();

private:
    double _epsilon;
    double _best_ub;
    int _nb_candidates;

    std::shared_ptr<SolverAbstract> _solver_model;

    void change_objective();
    void add_near_optimal_cost_constraint();
};