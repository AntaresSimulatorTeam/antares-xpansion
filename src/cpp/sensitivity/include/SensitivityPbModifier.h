#pragma once

#include <multisolver_interface/SolverAbstract.h>

#include "common.h"

class SensitivityPbModifier {
 public:
  SensitivityPbModifier() = default;
  explicit SensitivityPbModifier(double epsilon, double best_ubm, const std::shared_ptr<const SolverAbstract> &last_master);
  ~SensitivityPbModifier() = default;

  SolverAbstract::Ptr changeProblem(const int nb_candidates) const;

 private:
  const double _epsilon;
  const double _best_ub;
  const std::shared_ptr<const SolverAbstract> last_master;

  virtual std::vector<double> get_cost_vector(
      const const std::shared_ptr<const SolverAbstract> &solver_model, int nb_candidates) const = 0;
  void change_objective(
      const SolverAbstract::Ptr &solver_model,
      const std::vector<double> &obj) const;
  void add_near_optimal_cost_constraint(
      const SolverAbstract::Ptr &solver_model) const;
};