#include <numeric>
#include "PbModifierProjection.h"
#include "solver_utils.h"

PbModifierProjection::PbModifierProjection(double epsilon, double bestUb, int candidateId) : SensitivityPbModifier(epsilon, bestUb), _candidate_id(candidateId)
{
}

SolverAbstract::Ptr PbModifierProjection::change_objective(const SolverAbstract::Ptr &solverModel, int nbCandidates) const
{
    std::vector<int> colind(solverModel->get_ncols());
    std::vector<double> obj(colind.size(), 0);

    std::iota(std::begin(colind), std::end(colind), 0);

    // The objective is to find the min/max investment of candidate _candidate_id
    obj[_candidate_id] = 1;

    solverModel->chg_obj(colind, obj);
    return solverModel;
}