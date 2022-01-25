#include <numeric>
#include "SensitivityPbModifier.h"
#include "solver_utils.h"

SensitivityPbModifier::SensitivityPbModifier(double epsilon, double bestUb, std::map<int, std::string>& idToName, std::shared_ptr<SolverAbstract>& solverModel) : _epsilon(epsilon), _best_ub(bestUb), _solver_model(solverModel) {
    _nb_candidates = idToName.size();
}

std::shared_ptr<SolverAbstract> SensitivityPbModifier::getProblem()
{
    return _solver_model;
}

void SensitivityPbModifier::changeProblem()
{
    add_near_optimal_cost_constraint();
    change_objective();
}

void SensitivityPbModifier::change_objective()
{   
    std::vector<int> colind(_solver_model->get_ncols());
    std::vector<double> obj(colind.size());

    std::iota(std::begin(colind), std::end(colind), 0);

    solver_getobj(_solver_model, obj, 0, colind.size() - 1);

    //Keep only coefficients corresponding to candidates, alpha and all alpha_i are set to 0
    for (int i(_nb_candidates); i < obj.size(); i++)
    {
        obj[i] = 0;
    }
    _solver_model->chg_obj(colind, obj);
}

void SensitivityPbModifier::add_near_optimal_cost_constraint()
{
    std::vector<int> colind(_nb_candidates + 1);
    std::vector<double> dmatval(colind.size());
    std::vector<char> rowtype{'L'};
    std::vector<double> rhs{_best_ub + _epsilon};
    std::vector<int> rstart{0, _nb_candidates + 1};

    std::iota(std::begin(colind), std::end(colind), 0);
    
    solver_getobj(_solver_model, dmatval, 0, colind.size() - 1);
    solver_addrows(_solver_model, rowtype, rhs, {}, rstart, colind, dmatval);
    
    std::vector<std::basic_string<char>> col_names = _solver_model->get_col_names(0, _solver_model->get_ncols() - 1);
}