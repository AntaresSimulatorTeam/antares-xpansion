#include "SensitivityProblemModifier.h"

#include <numeric>
#include <utility>

#include "antares-xpansion/helpers/solver_utils.h"

SensitivityProblemModifier::SensitivityProblemModifier(
    double epsilon, double best_ub,
    std::shared_ptr<const SolverAbstract> last_master)
    : _epsilon(epsilon),
      _best_ub(best_ub),
      last_master(std::move(last_master)) {}

void change_objective(const SolverAbstract::Ptr &solver_model,
                      const std::vector<double> &obj) {
  std::vector<int> colind(solver_model->get_ncols());

  assert(colind.size() == obj.size());

  std::iota(std::begin(colind), std::end(colind), 0);

  solver_model->chg_obj(colind, obj);
}

SolverAbstract::Ptr SensitivityProblemModifier::changeProblem(
    unsigned int nb_candidates) const {
  SolverFactory factory;
  SolverAbstract::Ptr sensitivity_model = factory.copy_solver(last_master);
  std::vector<double> obj = get_cost_vector(*last_master, nb_candidates);

  add_near_optimal_cost_constraint(*(sensitivity_model.get()));
  change_objective(sensitivity_model, obj);

  return sensitivity_model;
}

void SensitivityProblemModifier::add_near_optimal_cost_constraint(
    SolverAbstract &solver_model) const {
  int ncols = solver_model.get_ncols();
  std::vector<int> colind(ncols);
  std::vector<double> dmatval(ncols);
  std::vector<char> rowtype{'L'};
  std::vector<double> rhs{_best_ub + _epsilon};
  std::vector<int> rstart{0, ncols};

  std::iota(std::begin(colind), std::end(colind), 0);

  solver_get_obj_func_coeffs(solver_model, dmatval, 0, colind.size() - 1);
  solver_addrows(solver_model, rowtype, rhs, {}, rstart, colind, dmatval);
}