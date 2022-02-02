#include <numeric>
#include "SensitivityPbModifier.h"
#include "solver_utils.h"

SensitivityPbModifier::SensitivityPbModifier(double epsilon, double best_ub) : _epsilon(epsilon), _best_ub(best_ub)
{
}

SolverAbstract::Ptr SensitivityPbModifier::changeProblem(const int nb_candidates, const SolverAbstract::Ptr &last_master)
{
    SolverFactory factory;
    SolverAbstract::Ptr sensitivity_model = factory.create_solver(last_master);

    sensitivity_model = add_near_optimal_cost_constraint(sensitivity_model, nb_candidates);
    sensitivity_model = change_objective(sensitivity_model, nb_candidates);

    return sensitivity_model;
}

SolverAbstract::Ptr SensitivityPbModifier::add_near_optimal_cost_constraint(const SolverAbstract::Ptr &solver_model, int nb_candidates) const
{
    std::vector<int> colind(nb_candidates + 1);
    std::vector<double> dmatval(colind.size());
    std::vector<char> rowtype{'L'};
    std::vector<double> rhs{_best_ub + _epsilon};
    std::vector<int> rstart{0, nb_candidates + 1};

    std::iota(std::begin(colind), std::end(colind), 0);

    solver_getobj(solver_model, dmatval, 0, colind.size() - 1);
    solver_addrows(solver_model, rowtype, rhs, {}, rstart, colind, dmatval);

    std::vector<std::basic_string<char>> col_names = solver_model->get_col_names(0, solver_model->get_ncols() - 1);
    return solver_model;
}