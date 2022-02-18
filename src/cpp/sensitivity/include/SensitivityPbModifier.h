#pragma once

#include "common.h"
#include <multisolver_interface/SolverAbstract.h>

class SensitivityPbModifier
{
public:
    SensitivityPbModifier() = default;
    explicit SensitivityPbModifier(double epsilon, double best_ub);
    ~SensitivityPbModifier() = default;

    SolverAbstract::Ptr changeProblem(const int nb_candidates, const SolverAbstract::Ptr &last_master);

private:
    double _epsilon;
    double _best_ub;

    virtual std::vector<double> get_cost_vector(const SolverAbstract::Ptr &solver_model, int nb_candidates) const = 0;
    SolverAbstract::Ptr change_objective(SolverAbstract::Ptr &solver_model, const std::vector<double> &obj) const;
    SolverAbstract::Ptr add_near_optimal_cost_constraint(const SolverAbstract::Ptr &solver_model, int nb_candidates) const;
};