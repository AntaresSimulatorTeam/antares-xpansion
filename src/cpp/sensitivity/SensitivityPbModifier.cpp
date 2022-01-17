#include "SensitivityPbModifier.h"

SensitivityPbModifier::SensitivityPbModifier() = default;

std::shared_ptr<SolverAbstract> SensitivityPbModifier::changeProblem(std::shared_ptr<SolverAbstract> solverModel){

    return solverModel;
}