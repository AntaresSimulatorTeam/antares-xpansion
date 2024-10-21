#pragma once

#include "SubproblemWorker.h"
#include "Worker.h"
/*!
 * \class SubproblemWorker
 * \brief Class daughter of Worker Class, build and manage a master problem
 */
class WorkerMaster;
typedef std::shared_ptr<WorkerMaster> WorkerMasterPtr;

class WorkerMaster : public Worker {
 public:
  explicit WorkerMaster(Logger logger);
  WorkerMaster(VariableMap const &variable_map,
               const std::filesystem::path &path_to_mps,
               const std::string &solver_name, int log_level,
               int subproblems_count,
               SolverLogManager&solver_log_manager,
               bool mps_has_alpha, Logger logger);
  ~WorkerMaster() override = default;

  void get(Point &x0, double &overall_subpb_cost_under_approx,
           DblVector &single_subpb_costs_under_approx);
  void get_dual_values(std::vector<double> &dual) const;
  [[nodiscard]] int get_number_constraint() const;

  void add_cut(Point const &s, Point const &x0, double const &rhs) const;
  void add_cut_by_iter(int i, Point const &s, double const &sx0,
                       double const &rhs) const;
  void add_dynamic_cut(Point const &s, double const &sx0,
                       double const &rhs) const;
  void addSubproblemCut(int i, Point const &s, Point const &x0,
                        double const &rhs) const;
  void fix_alpha(double const &bestUB) const;

  virtual void DeactivateIntegrityConstraints() const;
  virtual void ActivateIntegrityConstraints() const;
  [[nodiscard]] virtual std::vector<int> get_id_nb_units() const { return _id_nb_units; };

 private:
  std::vector<int> _id_nb_units;
  std::vector<int> id_single_subpb_costs_under_approx_;
  int _id_alpha = 0;
  int subproblems_count;
  bool _mps_has_alpha = false;
  void define_matval_mclind(const Point &s, std::vector<double> &matval,
                            std::vector<int> &mclind) const;

  void DefineRhsWithMasterVariable(const Point &s, const Point &x0,
                                   const double &rhs,
                                   std::vector<double> &rowrhs) const;

  void define_rhs_from_sx0(const double &sx0, const double &rhs,
                           std::vector<double> &rowrhs) const;

  void define_matval_mclind_for_index(int i, const Point &s,
                                      std::vector<double> &matval,
                                      std::vector<int> &mclind) const;
  void _set_upper_bounds() const;
  void _set_alpha_var();
  void _set_nb_units_var_ids();
};
