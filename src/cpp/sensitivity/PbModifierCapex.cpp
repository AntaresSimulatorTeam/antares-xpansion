#include <numeric>
#include "PbModifierCapex.h"
#include "solver_utils.h"

PbModifierCapex::PbModifierCapex(double epsilon, double best_ub) : SensitivityPbModifier(epsilon, best_ub)
{
}

std::vector<double> PbModifierCapex::get_cost_vector(const SolverAbstract::Ptr &solver_model, int nb_candidates) const
{
    std::vector<double> obj(solver_model->get_ncols());

    solver_get_obj_func_coeffs(solver_model, obj, 0, solver_model->get_ncols() - 1);

    //Keep only coefficients corresponding to candidates, alpha and all alpha_i are set to 0
    for (int i(nb_candidates); i < obj.size(); i++)
    {
        obj[i] = 0;
    }
    return obj;
}