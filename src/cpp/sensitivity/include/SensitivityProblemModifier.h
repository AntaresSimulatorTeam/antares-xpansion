#pragma once

#include <multisolver_interface/SolverAbstract.h>

#include "antares-xpansion/benders/benders_core/common.h"

class SensitivityProblemModifier {
 public:
  SensitivityProblemModifier() = delete;
  explicit SensitivityProblemModifier(
      double epsilon, double best_ub,
      std::shared_ptr<const SolverAbstract> last_master);
  ~SensitivityProblemModifier() = default;

  SolverAbstract::Ptr changeProblem(unsigned int nb_candidates) const;

 private:
  const double _epsilon;
  const double _best_ub;
  const std::shared_ptr<const SolverAbstract> last_master;

  virtual std::vector<double> get_cost_vector(
      const SolverAbstract &solver_model, unsigned int nb_candidates) const = 0;
  void add_near_optimal_cost_constraint(SolverAbstract &solver_model) const;
};
