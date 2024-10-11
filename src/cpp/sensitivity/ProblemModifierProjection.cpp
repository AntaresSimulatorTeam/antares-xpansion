#include "antares-xpansion/sensitivity/ProblemModifierProjection.h"

#include <numeric>
#include <utility>

#include "antares-xpansion/helpers/solver_utils.h"

ProblemModifierProjection::ProblemModifierProjection(
    double epsilon, double best_ub,
    const std::shared_ptr<const SolverAbstract> &last_master, int candidate_id,
    std::string candidate_name)
    : SensitivityProblemModifier(epsilon, best_ub, last_master),
      _candidate_id(candidate_id),
      _candidate_name(std::move(candidate_name)) {}

std::vector<double> ProblemModifierProjection::get_cost_vector(
    const SolverAbstract &solver_model, unsigned int nb_candidates) const {
  std::vector<double> obj(solver_model.get_ncols(), 0);

  // The objective is to find the min/max investment of candidate _candidate_id
  obj[_candidate_id] = 1;
  return obj;
}

std::string ProblemModifierProjection::get_candidate_name() const {
  return _candidate_name;
}