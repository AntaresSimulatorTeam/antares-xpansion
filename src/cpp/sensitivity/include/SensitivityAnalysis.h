#pragma once

#include <multisolver_interface/SolverAbstract.h>
#include "SensitivityLogger.h"
#include "SensitivityWriter.h"

class SensitivityAnalysis
{
    public:
        explicit SensitivityAnalysis();
        ~SensitivityAnalysis();
        void launch();
    
    private:
        std::shared_ptr<SolverAbstract> _math_problem;
};