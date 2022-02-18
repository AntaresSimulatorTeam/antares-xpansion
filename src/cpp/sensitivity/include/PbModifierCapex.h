#pragma once

#include "SensitivityPbModifier.h"

class PbModifierCapex : public SensitivityPbModifier
{
public:
    PbModifierCapex() = default;
    explicit PbModifierCapex(double epsilon, double best_ub);
    ~PbModifierCapex() = default;

private:
    virtual std::vector<double> get_cost_vector(const SolverAbstract::Ptr &solver_model, int nb_candidates) const override;
};