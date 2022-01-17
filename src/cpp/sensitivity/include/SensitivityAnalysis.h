#pragma once

#include <multisolver_interface/SolverAbstract.h>
#include "SensitivityPbModifier.h"
#include "SensitivityLogger.h"
#include "SensitivityWriter.h"

class SensitivityAnalysis
{
public:
    explicit SensitivityAnalysis(std::shared_ptr<SolverAbstract> &mathProblem, SensitivityPbModifier &pbModifier, SensitivityLogger &logger, SensitivityWriter writer);
    ~SensitivityAnalysis();

    void launch();

private:
    std::shared_ptr<SolverAbstract> _solver_model;

    SensitivityPbModifier _pb_modifier;
    SensitivityLogger _logger;
    SensitivityWriter _writer;

    
};