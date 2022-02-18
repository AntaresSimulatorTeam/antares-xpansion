#include <numeric>
#include "PbModifierProjection.h"
#include "solver_utils.h"

PbModifierProjection::PbModifierProjection(double epsilon, double best_ub, int candidate_id, std::string candidate_name) : SensitivityPbModifier(epsilon, best_ub), _candidate_id(candidate_id), _candidate_name(candidate_name)
{
}

std::vector<double> PbModifierProjection::get_cost_vector(const SolverAbstract::Ptr &solver_model, int nb_candidates) const
{
    std::vector<double> obj(solver_model->get_ncols(), 0);

    // The objective is to find the min/max investment of candidate _candidate_id
    obj[_candidate_id] = 1;
    return obj;
}

std::string PbModifierProjection::get_candidate_name()
{
    return _candidate_name;
}