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
  WorkerMaster();
  WorkerMaster(VariableMap const &variable_map,
               const std::filesystem::path &path_to_mps,
               const std::string &solver_name, const int log_level,
               int subproblems_count, const std::filesystem::path &log_name,
               bool mps_has_alpha);
  virtual ~WorkerMaster() = default;

  void get(Point &x0, double &alpha, DblVector &alpha_i);
  void get_dual_values(std::vector<double> &dual) const;
  int get_number_constraint() const;

  void add_cut(Point const &s, Point const &x0, double const &rhs) const;
  void add_cut_by_iter(int const i, Point const &s, double const &sx0,
                       double const &rhs) const;
  void add_dynamic_cut(Point const &s, double const &sx0,
                       double const &rhs) const;
  void addSubproblemCut(int i, Point const &s, Point const &x0,
                        double const &rhs) const;
  void fix_alpha(double const &bestUB) const;

  void deactivate_integrity_constraints() const;
  void activate_integrity_constraints() const;

 private:
  std::vector<int> _id_nb_units;
  std::vector<int> _id_alpha_i;
  int _id_alpha = 0;
  int subproblems_count;
  bool _mps_has_alpha = false;
  void define_matval_mclind(const Point &s, std::vector<double> &matval,
                            std::vector<int> &mclind) const;

  void define_rhs_with_master_variable(const Point &s, const Point &x0,
                                       const double &rhs,
                                       std::vector<double> &rowrhs) const;

  void define_rhs_from_sx0(const double &sx0, const double &rhs,
                           std::vector<double> &rowrhs) const;

  void define_matval_mclind_for_index(const int i, const Point &s,
                                      std::vector<double> &matval,
                                      std::vector<int> &mclind) const;
  void _set_upper_bounds() const;
  void _set_alpha_var();
  void _set_nb_units_var_ids();
};
