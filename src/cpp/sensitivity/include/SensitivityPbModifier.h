#pragma once

#include <multisolver_interface/SolverAbstract.h>

class SensitivityPbModifier{
    public:
        explicit SensitivityPbModifier();
        ~SensitivityPbModifier();

        std::shared_ptr<SolverAbstract> changeProblem(std::shared_ptr<SolverAbstract> solverModel);
    
    private:
        std::shared_ptr<SolverAbstract> _solver_model;
};