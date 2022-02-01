#include <numeric>
#include "PbModifierCapex.h"
#include "solver_utils.h"

PbModifierCapex::PbModifierCapex(double epsilon, double bestUb) : SensitivityPbModifier(epsilon, bestUb)
{
}

SolverAbstract::Ptr PbModifierCapex::change_objective(const SolverAbstract::Ptr &solverModel, int nbCandidates) const
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