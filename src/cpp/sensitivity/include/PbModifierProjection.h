#pragma once

#include "SensitivityPbModifier.h"

class PbModifierProjection : public SensitivityPbModifier
{
public:
    PbModifierProjection() = default;
    explicit PbModifierProjection(double epsilon, double bestUb, int candidateId);
    ~PbModifierProjection() = default;

private:
    double _candidate_id;

    virtual SolverAbstract::Ptr change_objective(const SolverAbstract::Ptr &solverModel, int nbCandidates) const override;
};