#pragma once

#include <multisolver_interface/SolverAbstract.h>
#include "SensitivityLogger.h"
#include "SensitivityWriter.h"
#include "SensitivityPbModifier.h"

class SensitivityAnalysis
{
public:
    explicit SensitivityAnalysis(std::shared_ptr<SolverAbstract> &math_problem, SensitivityLogger &logger, SensitivityWriter writer);
    ~SensitivityAnalysis();
    void launch();

private:
    std::shared_ptr<SolverAbstract> _math_problem;

    SensitivityPbModifier _pb_modifier;

    SensitivityLogger _logger;
    SensitivityWriter _writer;
};