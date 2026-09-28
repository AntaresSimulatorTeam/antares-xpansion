#pragma once

#include <fstream>
#include <memory>
#include <optional>
#include <ortools/math_opt/cpp/math_opt.h>
#include <string>
#include <unordered_map>
#include <vector>

#include "antares-xpansion/multisolver_interface/SolverAbstract.h"

namespace math_opt = ::operations_research::math_opt;

class SolverMathOpt: public SolverAbstract
{
public:
    SolverMathOpt();
    explicit SolverMathOpt(const SolverLogManager& log_manager);
    SolverMathOpt(const SolverMathOpt& other);
    SolverMathOpt& operator=(const SolverMathOpt&) = delete;
    ~SolverMathOpt() override;

    [[nodiscard]] SolverMathOpt* clone() const override;
    int get_number_of_instances() override;

    std::string get_solver_name() const override
    {
        return name_;
    }

    void init() override;
    void free() override;

    // Reading & Writing
    void write_prob_mps(const std::filesystem::path& filename) override;
    void write_prob_lp(const std::filesystem::path& filename) override;
    void save_prob(const std::filesystem::path& filename) override;
    void write_basis(const std::filesystem::path& filename) override;
    void read_prob_mps(const std::filesystem::path& filename) override;
    void read_prob_lp(const std::filesystem::path& filename) override;
    void restore_prob(const std::filesystem::path& filename) override;
    void read_basis(const std::filesystem::path& filename) override;
    void set_basis(std::span<int> rstatus, std::span<int> cstatus) override;

    // Problem info
    int get_ncols() const override;
    int get_nrows() const override;
    int get_nelems() const override;
    int get_n_integer_vars() const override;
    void get_obj(double* obj, int first, int last) const override;
    void set_obj_to_zero() override;
    void set_obj(const double* obj, int first, int last) override;
    void get_rows(int* mstart,
                  int* mclind,
                  double* dmatval,
                  int size,
                  int* nels,
                  int first,
                  int last) const override;
    void get_row_type(char* qrtype, int first, int last) const override;
    void get_rhs(double* rhs, int first, int last) const override;
    void get_rhs_range(double* range, int first, int last) const override;
    void get_cols(int* mstart,
                  int* mrwind,
                  double* dmatval,
                  int size,
                  int* nels,
                  int first,
                  int last) const override;
    void get_col_type(char* coltype, int first, int last) const override;
    void get_lb(double* lb, int first, int last) const override;
    void get_ub(double* ub, int first, int last) const override;

    int get_row_index(const std::string& name) override;
    int get_col_index(const std::string& name) override;
    std::vector<std::string> get_row_names(int first, int last) const override;
    std::vector<std::string> get_row_names() override;
    std::vector<std::string> get_col_names(int first, int last) const override;
    std::vector<std::string> get_col_names() override;

    // Problem modification
    void del_rows(int first, int last) override;
    void del_cols(int first, int last) override;
    void add_rows(int newrows,
                  int newnz,
                  const char* qrtype,
                  const double* rhs,
                  const double* range,
                  const int* mstart,
                  const int* mclind,
                  const double* dmatval,
                  const std::vector<std::string>& row_names) override;
    void add_cols(int newcol,
                  int newnz,
                  const double* objx,
                  const int* mstart,
                  const int* mrwind,
                  const double* dmatval,
                  const double* bdl,
                  const double* bdu,
                  const std::vector<std::string>& col_names) override;
    void add_name(int type, const char* cnames, int indice) override;
    void add_names(int type, const std::vector<std::string>& cnames, int first, int end) override;
    void chg_obj(const std::vector<int>& mindex, const std::vector<double>& obj) override;
    void chg_obj_direction(bool minimize) override;
    void chg_bounds(const std::vector<int>& mindex,
                    const std::vector<char>& qbtype,
                    const std::vector<double>& bnd) override;
    void chg_col_type(const std::vector<int>& mindex, const std::vector<char>& qctype) override;
    void chg_rhs(int id_row, double val) override;
    void chg_coef(int id_row, int id_col, double val) override;
    void chg_coefs(const std::vector<int>& id_rows,
                   const std::vector<int>& id_cols,
                   const std::vector<double>& vals) override;
    void chg_rhs_values(std::vector<int>&, std::vector<double>&) override;
    void chg_row_name(int id_row, const std::string& name) override;
    void chg_col_name(int id_col, const std::string& name) override;

    // Solving
    int solve_lp() override;
    int solve_mip() override;

    // Solution info
    void get_basis(int* rstatus, int* cstatus) const override;
    double get_mip_value() const override;
    double get_lp_value() const override;
    int get_splex_num_of_ite_last() const override;
    void get_lp_sol(double* primals, double* duals, double* reduced_costs) const override;
    void get_mip_sol(double* primals) override;
    void get_presolve_map(int* rowmap, int* colmap) const override;

    // Parameters
    void set_output_log_level(int loglevel) override;
    void set_algorithm(const std::string& algo) override;
    void set_threads(int n_threads) override;
    void set_optimality_gap(double gap) override;
    void set_simplex_iter(int iter) override;
    void mark_indices_to_keep_presolve(int nrows, int ncols, int* rowind, int* colind) override;
    void presolve_only() override;

private:
    const std::string name_ = "MATHOPT";
    std::unique_ptr<math_opt::Model> _model;
    std::vector<math_opt::Variable> _variables;
    std::vector<math_opt::LinearConstraint> _constraints;
    std::vector<char> _row_types;
    std::optional<math_opt::SolveResult> _last_result;
    std::optional<math_opt::Basis> _initial_basis;
    bool _minimize = true;
    int _log_level = 0;
    int _threads = 1;
    std::optional<int> _iteration_limit;
    std::ofstream _log_stream;

    void rebuild_from_model();
    void rebuild_index_maps();
    std::unordered_map<std::string, int> _col_name_to_index;
    std::unordered_map<std::string, int> _row_name_to_index;
};
