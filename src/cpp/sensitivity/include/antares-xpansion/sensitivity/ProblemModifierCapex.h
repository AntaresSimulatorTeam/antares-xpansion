#pragma once

#include "antares-xpansion/sensitivity/SensitivityProblemModifier.h"

class ProblemModifierCapex : public SensitivityProblemModifier {
public:
 ProblemModifierCapex() = delete;
    explicit ProblemModifierCapex(double epsilon, double best_ub, const std::shared_ptr<const SolverAbstract> &last_master);
    ~ProblemModifierCapex() = default;

private:
    std::vector<double> get_cost_vector(
        const SolverAbstract &solver_model,
        unsigned int nb_candidates) const override;
};