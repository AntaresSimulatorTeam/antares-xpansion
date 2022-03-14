#pragma once

#include "SensitivityPbModifier.h"

class PbModifierCapex : public SensitivityPbModifier
{
public:
    PbModifierCapex() = default;
    explicit PbModifierCapex(double epsilon, double best_ub, const std::shared_ptr<const SolverAbstract> &last_master);
    ~PbModifierCapex() = default;

private:
    std::vector<double> get_cost_vector(const std::shared_ptr<const SolverAbstract> &solver_model, int nb_candidates) const override;
};