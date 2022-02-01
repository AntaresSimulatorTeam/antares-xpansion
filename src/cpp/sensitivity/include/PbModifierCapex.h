#pragma once

#include "SensitivityPbModifier.h"

class PbModifierCapex : public SensitivityPbModifier
{
public:
    PbModifierCapex() = default;
    explicit PbModifierCapex(double epsilon, double bestUb);
    ~PbModifierCapex() = default;

private:
    virtual SolverAbstract::Ptr change_objective(const SolverAbstract::Ptr &solverModel, int nbCandidates) const override;
};