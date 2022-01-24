#pragma once

#include "common.h"
#include <multisolver_interface/SolverAbstract.h>

class SensitivityPbModifier
{
public:
    explicit SensitivityPbModifier(double epsilon, std::shared_ptr<BendersData>& bendersData, std::shared_ptr<SolverAbstract>& solverModel);
    ~SensitivityPbModifier() = default;

    std::shared_ptr<SolverAbstract> getProblem();
    void changeProblem();

private:
    double _epsilon;
    std::shared_ptr<BendersData> _benders_data;

    std::shared_ptr<SolverAbstract> _solver_model;

    void change_objective();
    void add_near_optimal_cost_constraint();
};