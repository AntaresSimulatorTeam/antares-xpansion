//
// Created by marechaljas on 02/05/2022.
//

#ifndef ANTARESXPANSION_TESTS_CPP_LP_NAMER_NOOPSOLVER_H_
#define ANTARESXPANSION_TESTS_CPP_LP_NAMER_NOOPSOLVER_H_

#include "antares-xpansion/multisolver_interface/SolverAbstract.h"

#include <algorithm>
#include <vector>
#include <map>

class NOOPSolver: public SolverAbstract
{
public:
    SolverAbstract* clone() const override
    {
        return new NOOPSolver(*this);
    }

    int get_number_of_instances() override
    {
        return 0;
    }

    std::string get_solver_name() const override
    {
        return std::string();
    }

    void init() override
    {
    }

    void free() override
    {
    }

    void write_prob_mps(const std::filesystem::path& filename) override
    {
    }

    void write_prob_lp(const std::filesystem::path& filename) override
    {
    }

    void read_prob_mps(const std::filesystem::path& filename) override
    {
    }

    void read_prob_lp(const std::filesystem::path& filename) override
    {
    }

    int get_ncols() const override
    {
        return 0;
    }

    int get_nrows() const override
    {
        return 0;
    }

    int get_nelems() const override
    {
        return 0;
    }

    int get_n_integer_vars() const override
    {
        return 0;
    }

    void get_obj(double* obj, int first, int last) const override
    {
    }

    void set_obj_to_zero() override
    {
    }

    void set_obj(const double* obj, int first, int last) override
    {
    }

    void get_rows(int* mstart,
                  int* mclind,
                  double* dmatval,
                  int size,
                  int* nels,
                  int first,
                  int last) const override
    {
    }

    void get_row_type(char* qrtype, int first, int last) const override
    {
    }

    void get_rhs(double* rhs, int first, int last) const override
    {
    }

    void get_rhs_range(double* range, int first, int last) const override
    {
    }

    void get_cols(int* mstart,
                  int* mrwind,
                  double* dmatval,
                  int size,
                  int* nels,
                  int first,
                  int last) const override
    {
    }

    void get_col_type(char* coltype, int first, int last) const override
    {
    }

    void get_lb(double* lb, int fisrt, int last) const override
    {
    }

    void get_ub(double* ub, int fisrt, int last) const override
    {
    }

    int get_row_index(const std::string& name) override
    {
        return 0;
    }

    int get_col_index(const std::string& name) override
    {
        return 0;
    }

    std::vector<std::string> get_row_names(int first, int last) const override
    {
        return std::vector<std::string>();
    }

    std::vector<std::string> get_row_names() override
    {
        return std::vector<std::string>();
    }

    std::vector<std::string> get_col_names(int first, int last) const override
    {
        return std::vector<std::string>();
    }

    std::vector<std::string> get_col_names() override
    {
        return std::vector<std::string>();
    }

    void del_rows(int first, int last) override
    {
    }

    void del_cols(int first, int last) override
    {
    }

    void add_rows(int newrows,
                  int newnz,
                  const char* qrtype,
                  const double* rhs,
                  const double* range,
                  const int* mstart,
                  const int* mclind,
                  const double* dmatval,
                  const std::vector<std::string>& row_names) override
    {
    }

    void add_cols(int newcol,
                  int newnz,
                  const double* objx,
                  const int* mstart,
                  const int* mrwind,
                  const double* dmatval,
                  const double* bdl,
                  const double* bdu,
                  const std::vector<std::string>& col_names) override
    {
    }

    void add_name(int type, const char* cnames, int indice) override
    {
    }

    void add_names(int type, const std::vector<std::string>& cnames, int first, int end) override
    {
    }

    void chg_obj(const std::vector<int>& mindex, const std::vector<double>& obj) override
    {
    }

    void chg_obj_direction(const bool minimize) override
    {
    }

    void chg_bounds(const std::vector<int>& mindex,
                    const std::vector<char>& qbtype,
                    const std::vector<double>& bnd) override
    {
    }

    void chg_col_type(const std::vector<int>& mindex, const std::vector<char>& qctype) override
    {
    }

    void chg_rhs(int id_row, double val) override
    {
    }

    void chg_coef(int id_row, int id_col, double val) override
    {
    }

    void chg_rhs_values(std::vector<int>& id_rows, std::vector<double>& vals) override
    {
    }

    void chg_coefs(const std::vector<int>& id_rows, const std::vector<int>& id_cols, const std::vector<double>& vals) override
    {
    }

    void chg_row_name(int id_row, const std::string& name) override
    {
    }

    void chg_col_name(int id_col, const std::string& name) override
    {
    }

    int solve_lp() override
    {
        return 0;
    }

    int solve_mip() override
    {
        return 0;
    }

    void get_basis(int* rstatus, int* cstatus) const override
    {
    }

    double get_mip_value() const override
    {
        return 0;
    }

    double get_lp_value() const override
    {
        return 0;
    }

    int get_splex_num_of_ite_last() const override
    {
        return 0;
    }

    void get_lp_sol(double* primals, double* duals, double* reduced_costs) const override
    {
    }

    void get_mip_sol(double* primals) override
    {
    }

    void set_output_log_level(int loglevel) override
    {
    }

    void set_algorithm(const std::string& algo) override
    {
    }

    void set_threads(int n_threads) override
    {
    }

    void set_optimality_gap(double gap) override
    {
    }

    void set_simplex_iter(int iter) override
    {
    }

    void write_basis(const std::filesystem::path& filename) override
    {
    }

    void read_basis(const std::filesystem::path& filename) override
    {
    }

    void set_basis(std::span<int> rstatus, std::span<int> cstatus) override {};

    void save_prob(const std::filesystem::path& filename) override
    {
    }

    void restore_prob(const std::filesystem::path& filename) override
    {
    }

    void get_presolve_map(int* rowmap, int* colmap) const override
    {
    }

    void mark_indices_to_keep_presolve(int nrows, int ncols, int* rowind, int* colind) override
    {
    }

    void presolve_only() override
    {
    }
};

#endif // ANTARESXPANSION_TESTS_CPP_LP_NAMER_NOOPSOLVER_H_

class NOOPSolverForWorker: public NOOPSolver
{
public:
    void get_col_type(char* coltype, int begin, int end) const override
    {
        std::ranges::copy_n(col_types.begin(), end - begin + 1, coltype);
    }

    void get_lb(double* lb, int begin, int end) const override
    {
        std::ranges::copy_n(lbs.begin(), end - begin + 1, lb);
    }

    void get_ub(double* ub, int begin, int end) const override
    {
        std::ranges::copy_n(ubs.begin(), end - begin + 1, ub);
    }

protected:
    std::vector<char> col_types;
    std::vector<double> lbs;
    std::vector<double> ubs;
};


 
class NOOPSolverForSkeleton: public NOOPSolver
{
public:
    NOOPSolverForSkeleton() 
    {
        col_name_index_map_["x1"] = 0 ; 
        col_name_index_map_["x2"] = 1 ; 
        row_name_index_map_["row1"] = 0 ; 
        row_name_index_map_["row2"] = 1 ; 


    }
    int get_ncols() const override
    {
        return ncols_;
    }

    void set_ncols(int ncols)
    {
        ncols_ = ncols;
        obj_coeffs_.resize(ncols);
    }

    int get_nrows() const override
    {
        return nrows_;
    }

    void set_nrows(int nrows)
    {
        nrows_ = nrows;
        rhs_coeffs_.resize(nrows);
        constraints_coeffs_.resize(nrows);
    }

    void set_obj(const double* obj, int first, int last) override
    {
        std::copy(obj, obj + (last - first + 1), obj_coeffs_.begin() + first);
    }

    void set_rhs(const std::vector<double>& rhs)
    {
        rhs_coeffs_ = rhs;
    }

    void set_constraints(const std::vector<std::vector<double>>& constraints)
    {
        constraints_coeffs_ = constraints;
    }

    void chg_obj(const std::vector<int>& mindex, const std::vector<double>& obj) override
    {
        for (size_t i = 0; i < obj.size(); ++i)
        {
            obj_coeffs_[mindex[i]] = obj[i];
        }
    }

    void chg_rhs_values(std::vector<int>& id_rows, std::vector<double>& vals) override
    {
        for (size_t i = 0; i < vals.size(); ++i)
        {
            rhs_coeffs_[id_rows[i]] = vals[i];
        }
    }

    void chg_coefs(const std::vector<int>& id_rows,
                   const std::vector<int>& id_cols,
                   const std::vector<double>& vals) override
    {
        for (size_t i = 0; i < vals.size(); ++i)
        {
            constraints_coeffs_[id_rows[i]][id_cols[i]] = vals[i];
        }
    }

    void get_obj(double* obj, int first, int last) const override
    {
        std::copy(obj_coeffs_.begin() + first, obj_coeffs_.begin() + last + 1, obj);
    }

    void get_rhs(double* rhs, int first, int last) const override
    {
        std::copy(rhs_coeffs_.begin() + first, rhs_coeffs_.begin() + last + 1, rhs);
    }

    int get_col_index(const std::string& name) override
    {
        auto it = col_name_index_map_.find(name);
        return it == col_name_index_map_.end() ? -1 : it->second;
    }

    int get_row_index(const std::string& name) override
    {
        auto it = row_name_index_map_.find(name);
        return it == row_name_index_map_.end() ? -1 : it->second;
    }

    void get_rows(int* mstart,
                  int* mclind,
                  double* dmatval,
                  int /*size*/,
                  int* nels,
                  int first,
                  int last) const override
    {
        int nz = 0;
        for (int r = first; r <= last; ++r)
        {
            mstart[r - first] = nz;
            const auto& row = constraints_coeffs_[r];
            for (size_t c = 0; c < row.size(); ++c)
            {
                mclind[nz] = static_cast<int>(c);
                dmatval[nz] = row[c];
                ++nz;
            }
            nels[r - first] = static_cast<int>(row.size());
        }
    }

    int ncols_ = 0;
    int nrows_ = 0;
    std::vector<double> obj_coeffs_;
    std::vector<double> rhs_coeffs_;
    std::vector<std::vector<double>> constraints_coeffs_;
    std::map<std::string,int> col_name_index_map_ ; 
    std::map<std::string,int> row_name_index_map_ ; 
};
