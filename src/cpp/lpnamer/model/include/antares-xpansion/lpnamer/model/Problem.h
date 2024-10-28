//
// Created by marechaljas on 27/04/2022.
//

#ifndef ANTARESXPANSION_SRC_CPP_LPNAMER_MODEL_PROBLEM_H_
#define ANTARESXPANSION_SRC_CPP_LPNAMER_MODEL_PROBLEM_H_

#include <utility>

#include "antares-xpansion/multisolver_interface/SolverAbstract.h"

/**
 * @brief Decorator to the SolverAbstract class
 */
class Problem : public SolverAbstract {
 public:
  Problem() = delete;
  explicit Problem(SolverAbstract::Ptr solver_abstract)
      : solver_abstract_(std::move(solver_abstract)) {}

 private:
  const SolverAbstract::Ptr solver_abstract_;

 public:
  [[nodiscard]] unsigned int McYear() const { return mc_year; }
  [[nodiscard]] unsigned int Week() const { return week; }

  unsigned int mc_year = 0;
  unsigned int week = 0;

  int get_number_of_instances() override {
    return solver_abstract_->get_number_of_instances();
  }
  [[nodiscard]] std::string get_solver_name() const override {
    return solver_abstract_->get_solver_name();
  }
  void init() override { solver_abstract_->init(); }
  void free() override { solver_abstract_->free(); }
  void write_prob_mps(const std::filesystem::path &filename) override {
    solver_abstract_->write_prob_mps(filename);
  }
  void write_prob_lp(const std::filesystem::path &filename) override {
    solver_abstract_->write_prob_lp(filename);
  }
  void read_prob_mps(const std::filesystem::path &filename) override;
  void read_prob_lp(const std::filesystem::path &filename) override {
    solver_abstract_->read_prob_lp(filename);
  }
  void copy_prob(Ptr fictif_solv) override {
    solver_abstract_->copy_prob(fictif_solv);
  }
  [[nodiscard]] int get_ncols() const override {
    return solver_abstract_->get_ncols();
  }
  [[nodiscard]] int get_nrows() const override {
    return solver_abstract_->get_nrows();
  }
  [[nodiscard]] int get_nelems() const override {
    return solver_abstract_->get_nelems();
  }
  [[nodiscard]] int get_n_integer_vars() const override {
    return solver_abstract_->get_n_integer_vars();
  }
  void get_obj(double *obj, int first, int last) const override {
    solver_abstract_->get_obj(obj, first, last);
  }

  void set_obj_to_zero() override { solver_abstract_->set_obj_to_zero(); }
  void set_obj(const double *obj, int first, int last) override {
    solver_abstract_->set_obj(obj, first, last);
  }
  void get_rows(int *mstart, int *mclind, double *dmatval, int size, int *nels,
                int first, int last) const override {
    solver_abstract_->get_rows(mstart, mclind, dmatval, size, nels, first,
                               last);
  }
  void get_row_type(char *qrtype, int first, int last) const override {
    solver_abstract_->get_row_type(qrtype, first, last);
  }
  void get_rhs(double *rhs, int first, int last) const override {
    solver_abstract_->get_rhs(rhs, first, last);
  }
  void get_rhs_range(double *range, int first, int last) const override {
    solver_abstract_->get_rhs_range(range, first, last);
  }
  void get_col_type(char *coltype, int first, int last) const override {
    solver_abstract_->get_col_type(coltype, first, last);
  }
  void get_lb(double *lb, int fisrt, int last) const override {
    solver_abstract_->get_lb(lb, fisrt, last);
  }
  void get_ub(double *ub, int fisrt, int last) const override {
    solver_abstract_->get_ub(ub, fisrt, last);
  }
  [[nodiscard]] int get_row_index(const std::string &name) override {
    return solver_abstract_->get_row_index(name);
  }
  [[nodiscard]] int get_col_index(const std::string &name) override {
    return solver_abstract_->get_col_index(name);
  }
  std::vector<std::string> get_row_names(int first, int last) override {
    return solver_abstract_->get_row_names(first, last);
  }
  std::vector<std::string> get_row_names() override {
    return solver_abstract_->get_row_names();
  }
  std::vector<std::string> get_col_names(int first, int last) override {
    return solver_abstract_->get_col_names(first, last);
  }
  std::vector<std::string> get_col_names() override {
    return solver_abstract_->get_col_names();
  }
  void del_rows(int first, int last) override {
    solver_abstract_->del_rows(first, last);
  }
  void add_rows(int newrows, int newnz, const char *qrtype, const double *rhs,
                const double *range, const int *mstart, const int *mclind,
                const double *dmatval,
                const std::vector<std::string> &names = {}) override {
    solver_abstract_->add_rows(newrows, newnz, qrtype, rhs, range, mstart,
                               mclind, dmatval, names);
  }
  void add_cols(int newcol, int newnz, const double *objx, const int *mstart,
                const int *mrwind, const double *dmatval, const double *bdl,
                const double *bdu) override {
    solver_abstract_->add_cols(newcol, newnz, objx, mstart, mrwind, dmatval,
                               bdl, bdu);
  }
  void add_name(int type, const char *cnames, int indice) override {
    solver_abstract_->add_name(type, cnames, indice);
  }
  void add_names(int type, const std::vector<std::string> &cnames, int first,
                 int end) override {
    solver_abstract_->add_names(type, cnames, first, end);
  }
  void chg_obj(const std::vector<int> &mindex,
               const std::vector<double> &obj) override {
    solver_abstract_->chg_obj(mindex, obj);
  }
  void chg_obj_direction(const bool minimize) override {
    solver_abstract_->chg_obj_direction(minimize);
  }
  void chg_bounds(const std::vector<int> &mindex,
                  const std::vector<char> &qbtype,
                  const std::vector<double> &bnd) override {
    solver_abstract_->chg_bounds(mindex, qbtype, bnd);
  }
  void chg_col_type(const std::vector<int> &mindex,
                    const std::vector<char> &qctype) override {
    solver_abstract_->chg_col_type(mindex, qctype);
  }
  void chg_rhs(int id_row, double val) override {
    solver_abstract_->chg_rhs(id_row, val);
  }
  void chg_coef(int id_row, int id_col, double val) override {
    solver_abstract_->chg_coef(id_row, id_col, val);
  }
  void chg_row_name(int id_row, const std::string &name) override {
    solver_abstract_->chg_row_name(id_row, name);
  }
  void chg_col_name(int id_col, const std::string &name) override {
    solver_abstract_->chg_col_name(id_col, name);
  }
  int solve_lp() override { return solver_abstract_->solve_lp(); }
  int solve_mip() override { return solver_abstract_->solve_mip(); }
  void get_basis(int *rstatus, int *cstatus) const override {
    solver_abstract_->get_basis(rstatus, cstatus);
  }
  [[nodiscard]] double get_mip_value() const override {
    return solver_abstract_->get_mip_value();
  }
  [[nodiscard]] double get_lp_value() const override {
    return solver_abstract_->get_lp_value();
  }
  [[nodiscard]] int get_splex_num_of_ite_last() const override {
    return solver_abstract_->get_splex_num_of_ite_last();
  }
  void get_lp_sol(double *primals, double *duals,
                  double *reduced_costs) override {
    solver_abstract_->get_lp_sol(primals, duals, reduced_costs);
  }
  void get_mip_sol(double *primals) override {
    solver_abstract_->get_mip_sol(primals);
  }
  void set_output_log_level(int loglevel) override {
    solver_abstract_->set_output_log_level(loglevel);
  }
  void set_algorithm(const std::string &algo) override {
    solver_abstract_->set_algorithm(algo);
  }
  void set_threads(int n_threads) override {
    solver_abstract_->set_threads(n_threads);
  }
  void set_optimality_gap(double gap) override {
    solver_abstract_->set_optimality_gap(gap);
  }
  void set_simplex_iter(int iter) override {
    solver_abstract_->set_simplex_iter(iter);
  }
  void write_basis(const std::filesystem::path &filename) override {
    solver_abstract_->write_basis(filename);
  }
  void read_basis(const std::filesystem::path &filename) override {
    solver_abstract_->read_basis(filename);
  }
};

#endif  // ANTARESXPANSION_SRC_CPP_LPNAMER_MODEL_PROBLEM_H_
