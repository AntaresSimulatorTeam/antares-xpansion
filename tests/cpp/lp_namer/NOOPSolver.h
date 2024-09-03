//
// Created by marechaljas on 02/05/2022.
//

#ifndef ANTARESXPANSION_TESTS_CPP_LP_NAMER_NOOPSOLVER_H_
#define ANTARESXPANSION_TESTS_CPP_LP_NAMER_NOOPSOLVER_H_

#include "multisolver_interface/SolverAbstract.h"
class NOOPSolver: public SolverAbstract {
 public:
  virtual int get_number_of_instances() override { return 0; }
  virtual std::string get_solver_name() const override { return std::string(); }
  virtual void init() override {}
  virtual void free() override {}
  virtual void write_prob_mps(const std::filesystem::path &filename) override {}
  virtual void write_prob_lp(const std::filesystem::path &filename) override {}
  virtual void read_prob_mps(const std::filesystem::path &filename) override {}
  virtual void read_prob_lp(const std::filesystem::path &filename) override {}
  virtual void copy_prob(Ptr fictif_solv) override {}
  virtual int get_ncols() const override { return 0; }
  virtual int get_nrows() const override { return 0; }
  virtual int get_nelems() const override { return 0; }
  virtual int get_n_integer_vars() const override { return 0; }
  virtual void get_obj(double *obj, int first, int last) const override {}
  void set_obj_to_zero() override {}
  void set_obj(const double *obj, int first, int last) override {}
  virtual void get_rows(int *mstart, int *mclind, double *dmatval, int size,
                        int *nels, int first, int last) const override {}
  virtual void get_row_type(char *qrtype, int first, int last) const override {}
  virtual void get_rhs(double *rhs, int first, int last) const override {}
  virtual void get_rhs_range(double *range, int first,
                             int last) const override {}
  virtual void get_col_type(char *coltype, int first, int last) const override {

  }
  virtual void get_lb(double *lb, int fisrt, int last) const override {}
  virtual void get_ub(double *ub, int fisrt, int last) const override {}
  virtual int get_row_index(const std::string &name) override { return 0; }
  virtual int get_col_index(const std::string &name) override { return 0; }
  virtual std::vector<std::string> get_row_names(int first, int last) override {
    return std::vector<std::string>();
  }
  virtual std::vector<std::string> get_row_names() override {
    return std::vector<std::string>();
  }
  virtual std::vector<std::string> get_col_names(int first, int last) override {
    return std::vector<std::string>();
  }
  virtual std::vector<std::string> get_col_names() override {
    return std::vector<std::string>();
  }
  virtual void del_rows(int first, int last) override {}
  virtual void add_rows(int newrows, int newnz, const char *qrtype,
                        const double *rhs, const double *range,
                        const int *mstart, const int *mclind,
                        const double *dmatval,
                        const std::vector<std::string> &names = {}) override {}
  virtual void add_cols(int newcol, int newnz, const double *objx,
                        const int *mstart, const int *mrwind,
                        const double *dmatval, const double *bdl,
                        const double *bdu) override {}
  virtual void add_name(int type, const char *cnames, int indice) override {}
  virtual void add_names(int type, const std::vector<std::string> &cnames,
                         int first, int end) override {}
  virtual void chg_obj(const std::vector<int> &mindex,
                       const std::vector<double> &obj) override {}
  virtual void chg_obj_direction(const bool minimize) override {}
  virtual void chg_bounds(const std::vector<int> &mindex,
                          const std::vector<char> &qbtype,
                          const std::vector<double> &bnd) override {}
  virtual void chg_col_type(const std::vector<int> &mindex,
                            const std::vector<char> &qctype) override {}
  virtual void chg_rhs(int id_row, double val) override {}
  virtual void chg_coef(int id_row, int id_col, double val) override {}
  virtual void chg_row_name(int id_row, const std::string &name) override {}
  virtual void chg_col_name(int id_col, const std::string &name) override {}
  virtual int solve_lp() override { return 0; }
  virtual int solve_mip() override { return 0; }
  virtual void get_basis(int *rstatus, int *cstatus) const override {}
  virtual double get_mip_value() const override { return 0; }
  virtual double get_lp_value() const override { return 0; }
  virtual int get_splex_num_of_ite_last() const override { return 0; }
  virtual void get_lp_sol(double *primals, double *duals,
                          double *reduced_costs) override {}
  virtual void get_mip_sol(double *primals) override {}
  virtual void set_output_log_level(int loglevel) override {}
  virtual void set_algorithm(const std::string &algo) override {}
  virtual void set_threads(int n_threads) override {}
  virtual void set_optimality_gap(double gap) override {}
  virtual void set_simplex_iter(int iter) override {}
  virtual void write_basis(const std::filesystem::path &filename) override {}
  virtual void read_basis(const std::filesystem::path &filename) override {}
  void SetBasis(std::vector<int> rstatus, std::vector<int> cstatus) override {}
};

#endif  // ANTARESXPANSION_TESTS_CPP_LP_NAMER_NOOPSOLVER_H_
