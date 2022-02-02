#pragma once

#include "SensitivityPbModifier.h"

class PbModifierProjection : public SensitivityPbModifier
{
public:
    PbModifierProjection() = default;
    explicit PbModifierProjection(double epsilon, double best_ub, int candidate_id);
    ~PbModifierProjection() = default;

private:
    double _candidate_id;

    virtual SolverAbstract::Ptr change_objective(const SolverAbstract::Ptr &solver_model, int nb_candidates) const override;
};