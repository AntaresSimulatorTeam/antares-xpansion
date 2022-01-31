#include <numeric>
#include "SensitivityPbModifier.h"
#include "solver_utils.h"

SensitivityPbModifier::SensitivityPbModifier(double epsilon, double bestUb) : _epsilon(epsilon), _best_ub(bestUb)
{
}

SolverAbstract::Ptr SensitivityPbModifier::changeProblem(const std::map<int, std::string> &idToName, const SolverAbstract::Ptr &lastMaster)
{
    SolverFactory factory;
    SolverAbstract::Ptr sensitivity_model = factory.create_solver(lastMaster);
    int nb_candidates = idToName.size();

    sensitivity_model = add_near_optimal_cost_constraint(sensitivity_model, nb_candidates);
    sensitivity_model = change_objective(sensitivity_model, nb_candidates);
    return sensitivity_model;
}

SolverAbstract::Ptr SensitivityPbModifier::change_objective(const SolverAbstract::Ptr &solverModel, int nbCandidates) const
{
    std::vector<int> colind(solverModel->get_ncols());
    std::vector<double> obj(colind.size());

    std::iota(std::begin(colind), std::end(colind), 0);

    solver_getobj(solverModel, obj, 0, colind.size() - 1);

    //Keep only coefficients corresponding to candidates, alpha and all alpha_i are set to 0
    for (int i(nbCandidates); i < obj.size(); i++)
    {
        obj[i] = 0;
    }
    solverModel->chg_obj(colind, obj);
    return solverModel;
}

SolverAbstract::Ptr SensitivityPbModifier::add_near_optimal_cost_constraint(const SolverAbstract::Ptr &solverModel, int nbCandidates)
{
    std::vector<int> colind(nbCandidates + 1);
    std::vector<double> dmatval(colind.size());
    std::vector<char> rowtype{'L'};
    std::vector<double> rhs{_best_ub + _epsilon};
    std::vector<int> rstart{0, nbCandidates + 1};

    std::iota(std::begin(colind), std::end(colind), 0);

    solver_getobj(solverModel, dmatval, 0, colind.size() - 1);
    solver_addrows(solverModel, rowtype, rhs, {}, rstart, colind, dmatval);

    std::vector<std::basic_string<char>> col_names = solverModel->get_col_names(0, solverModel->get_ncols() - 1);
    return solverModel;
}