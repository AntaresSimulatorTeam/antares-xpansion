#pragma once

#include "SensitivityPbModifier.h"

class PbModifierProjection : public SensitivityPbModifier
{
public:
    PbModifierProjection() = default;
    explicit PbModifierProjection(double epsilon, double best_ub, const std::shared_ptr<const SolverAbstract> &last_master, int candidate_id, const std::string &candidate_name);
    ~PbModifierProjection() = default;

    std::string get_candidate_name() const;

private:
    int _candidate_id;
    std::string _candidate_name;

    std::vector<double> get_cost_vector(const std::shared_ptr<const SolverAbstract> &solver_model, int nb_candidates) const override;
};