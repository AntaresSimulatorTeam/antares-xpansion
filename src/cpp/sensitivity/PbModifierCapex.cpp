#include <numeric>
#include "PbModifierCapex.h"
#include "solver_utils.h"

PbModifierCapex::PbModifierCapex(double epsilon, double best_ub) : SensitivityPbModifier(epsilon, best_ub)
{
}

SolverAbstract::Ptr PbModifierCapex::change_objective(const SolverAbstract::Ptr &solver_model, int nb_candidates) const
{
    std::vector<int> colind(solver_model->get_ncols());
    std::vector<double> obj(colind.size());

    std::iota(std::begin(colind), std::end(colind), 0);

    solver_getobj(solver_model, obj, 0, colind.size() - 1);

    //Keep only coefficients corresponding to candidates, alpha and all alpha_i are set to 0
    for (int i(nb_candidates); i < obj.size(); i++)
    {
        obj[i] = 0;
    }
    solver_model->chg_obj(colind, obj);
    return solver_model;
}