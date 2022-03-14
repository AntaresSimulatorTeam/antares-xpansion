#include <numeric>
#include "ProblemModifierCapex.h"
#include "solver_utils.h"

ProblemModifierCapex::ProblemModifierCapex(double epsilon, double best_ub, const std::shared_ptr<const SolverAbstract> &last_master) : SensitivityProblemModifier(epsilon, best_ub, last_master)
{
}

std::vector<double> ProblemModifierCapex::get_cost_vector(const std::shared_ptr<const SolverAbstract> &solver_model,
    unsigned int nb_candidates) const
{
    std::vector<double> obj(solver_model->get_ncols());

    solver_get_obj_func_coeffs(solver_model, obj, 0, solver_model->get_ncols() - 1);

    //Keep only coefficients corresponding to candidates, alpha and all alpha_i are set to 0
    for (unsigned int i(nb_candidates); i < obj.size(); i++)
    {
        obj[i] = 0;
    }
    return obj;
}