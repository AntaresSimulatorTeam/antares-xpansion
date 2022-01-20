#pragma once

#include <multisolver_interface/SolverAbstract.h>

class SensitivityPbModifier
{
public:
    explicit SensitivityPbModifier(double epsilon);
    ~SensitivityPbModifier();

    std::shared_ptr<SolverAbstract> changeProblem(std::shared_ptr<SolverAbstract> solverModel);

private:
    double _epsilon;
    std::shared_ptr<SolverAbstract> _solver_model;

    void change_objective();
    void add_near_optimal_cost_constraint(std::shared_ptr<SolverAbstract> soolverModel);
};