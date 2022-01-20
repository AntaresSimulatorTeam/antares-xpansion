#include "SensitivityPbModifier.h"
#include "solver_utils.h"

SensitivityPbModifier::SensitivityPbModifier(double epsilon) : _epsilon(epsilon) {}

std::shared_ptr<SolverAbstract> SensitivityPbModifier::changeProblem(std::shared_ptr<SolverAbstract> solverModel)
{
    add_near_optimal_cost_constraint(solverModel);
    return solverModel;
}

void SensitivityPbModifier::change_objective()
{
}

void SensitivityPbModifier::add_near_optimal_cost_constraint(std::shared_ptr<SolverAbstract> solverModel)
{

    std::vector<double> dmatval;
    std::vector<int> colind;
    std::vector<char> rowtype{'L'};
    std::vector<double> rhs;
    std::vector<int> rstart{0};

    solver_addrows(solverModel, rowtype, rhs, {}, rstart, colind, dmatval);
}