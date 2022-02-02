#include <numeric>
#include "PbModifierProjection.h"
#include "solver_utils.h"

PbModifierProjection::PbModifierProjection(double epsilon, double best_ub, int candidate_id) : SensitivityPbModifier(epsilon, best_ub), _candidate_id(candidate_id)
{
}

SolverAbstract::Ptr PbModifierProjection::change_objective(const SolverAbstract::Ptr &solver_model, int nb_candidates) const
{
    std::vector<int> colind(solver_model->get_ncols());
    std::vector<double> obj(colind.size(), 0);

    std::iota(std::begin(colind), std::end(colind), 0);

    // The objective is to find the min/max investment of candidate _candidate_id
    obj[_candidate_id] = 1;

    solver_model->chg_obj(colind, obj);
    return solver_model;
}