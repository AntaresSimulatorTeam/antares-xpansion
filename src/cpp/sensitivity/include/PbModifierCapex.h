#pragma once

#include "SensitivityPbModifier.h"

class PbModifierCapex : public SensitivityPbModifier
{
public:
    PbModifierCapex() = default;
    explicit PbModifierCapex(double epsilon, double best_ub);
    ~PbModifierCapex() = default;

private:
    virtual SolverAbstract::Ptr change_objective(const SolverAbstract::Ptr &solver_model, int nb_candidates) const override;
};