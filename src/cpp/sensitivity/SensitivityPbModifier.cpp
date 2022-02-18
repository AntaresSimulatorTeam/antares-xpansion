#include <numeric>
#include "SensitivityPbModifier.h"
#include "solver_utils.h"

SensitivityPbModifier::SensitivityPbModifier(double epsilon, double best_ub) : _epsilon(epsilon), _best_ub(best_ub)
{
}

SolverAbstract::Ptr SensitivityPbModifier::change_objective(SolverAbstract::Ptr &solver_model, const std::vector<double> &obj) const
{
    std::vector<int> colind(solver_model->get_ncols());

    assert(colind.size() == obj.size());

    std::iota(std::begin(colind), std::end(colind), 0);

    solver_model->chg_obj(colind, obj);
    return solver_model;
}

SolverAbstract::Ptr SensitivityPbModifier::changeProblem(const int nb_candidates, const SolverAbstract::Ptr &last_master)
{
    SolverFactory factory;
    SolverAbstract::Ptr sensitivity_model = factory.create_solver(last_master);
    std::vector<double> obj = get_cost_vector(last_master, nb_candidates);

    sensitivity_model = add_near_optimal_cost_constraint(sensitivity_model, nb_candidates);
    sensitivity_model = change_objective(sensitivity_model, obj);

    return sensitivity_model;
}

SolverAbstract::Ptr SensitivityPbModifier::add_near_optimal_cost_constraint(const SolverAbstract::Ptr &solver_model, int nb_candidates) const
{
    int ncols = solver_model->get_ncols();
    std::vector<int> colind(ncols);
    std::vector<double> dmatval(ncols);
    std::vector<char> rowtype{'L'};
    std::vector<double> rhs{_best_ub + _epsilon};
    std::vector<int> rstart{0, ncols};

    std::iota(std::begin(colind), std::end(colind), 0);

    solver_getobj(solver_model, dmatval, 0, colind.size() - 1);
    solver_addrows(solver_model, rowtype, rhs, {}, rstart, colind, dmatval);

    return solver_model;
}