#pragma once

#include "antares-xpansion/sensitivity/SensitivityProblemModifier.h"

class ProblemModifierProjection : public SensitivityProblemModifier {
public:
 ProblemModifierProjection() = delete;
    explicit ProblemModifierProjection(double epsilon, double best_ub, const std::shared_ptr<const SolverAbstract> &last_master, int candidate_id, std::string candidate_name);
    ~ProblemModifierProjection() = default;

    std::string get_candidate_name() const;

private:
    int _candidate_id;
    std::string _candidate_name;

    std::vector<double> get_cost_vector(
        const SolverAbstract &solver_model,
        unsigned int nb_candidates) const override;
};