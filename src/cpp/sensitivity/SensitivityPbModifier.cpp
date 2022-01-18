#include "SensitivityPbModifier.h"

SensitivityPbModifier::SensitivityPbModifier() = default;

std::shared_ptr<SolverAbstract> SensitivityPbModifier::changeProblem(std::shared_ptr<SolverAbstract> solverModel){

    return solverModel;
}

void SensitivityPbModifier::change_objective(){

}

void SensitivityPbModifier::add_near_optimal_cost_constraint(){
    
}