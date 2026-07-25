#pragma once

#include <algorithm>
#include <span>
#include <string>
#include <vector>

#include "NOOPSolver.h"

// Test-only solver double that returns configurable, deterministic values for
// row-marshalling queries and records the arguments of mutating calls, so
// tests can assert on data flowing through SolverRowExtractor and
// SkeletonConstraintCoefficients without relying on NOOPSolver's no-op
// (and, for get_row_type, uninitialized-buffer) behavior.
class RecordingSolver: public NOOPSolver
{
public:
    int ncols = 3;
    int row_index = 0;
    std::vector<int> row_mclind = {0, 1};
    std::vector<double> row_dmatval = {1.0, 2.0};
    double row_rhs = 42.0;
    double row_range = 0.0;
    char row_qrtype = 'L';

    int get_row_index(const std::string& /*name*/) override
    {
        return row_index;
    }

    int get_ncols() const override
    {
        return ncols;
    }

    void get_rows(int* mstart,
                  int* mclind,
                  double* dmatval,
                  int /*size*/,
                  int* nels,
                  int /*first*/,
                  int /*last*/) const override
    {
        mstart[0] = 0;
        *nels = static_cast<int>(row_mclind.size());
        std::copy(row_mclind.begin(), row_mclind.end(), mclind);
        std::copy(row_dmatval.begin(), row_dmatval.end(), dmatval);
    }

    void get_rhs(double* rhs, int /*first*/, int /*last*/) const override
    {
        *rhs = row_rhs;
    }

    void get_rhs_range(double* range, int /*first*/, int /*last*/) const override
    {
        *range = row_range;
    }

    void get_row_type(char* qrtype, int /*first*/, int /*last*/) const override
    {
        qrtype[0] = row_qrtype;
    }

    std::vector<int> chg_coefs_row_indices;
    std::vector<int> chg_coefs_col_indices;
    std::vector<double> chg_coefs_vals;
    int chg_coefs_calls = 0;

    void chg_coefs(const std::vector<int>& id_rows,
                   const std::vector<int>& id_cols,
                   const std::vector<double>& vals) override
    {
        chg_coefs_row_indices = id_rows;
        chg_coefs_col_indices = id_cols;
        chg_coefs_vals = vals;
        ++chg_coefs_calls;
    }

    std::vector<int> chg_rhs_rows;
    std::vector<double> chg_rhs_vals;
    int chg_rhs_values_calls = 0;

    void chg_rhs_values(std::vector<int>& id_rows, std::vector<double>& vals) override
    {
        chg_rhs_rows = id_rows;
        chg_rhs_vals = vals;
        ++chg_rhs_values_calls;
    }

    int nrows = 0;

    int get_nrows() const override
    {
        return nrows;
    }

    int del_rows_first = -1;
    int del_rows_last = -1;
    int del_rows_calls = 0;

    void del_rows(int first, int last) override
    {
        del_rows_first = first;
        del_rows_last = last;
        ++del_rows_calls;
    }

    int add_rows_calls = 0;

    void add_rows(int /*newrows*/,
                  int /*newnz*/,
                  const char* /*qrtype*/,
                  const double* /*rhs*/,
                  const double* /*range*/,
                  const int* /*mstart*/,
                  const int* /*mclind*/,
                  const double* /*dmatval*/,
                  const std::vector<std::string>& /*row_names*/) override
    {
        ++add_rows_calls;
    }

    std::vector<int> basis_rstatus_out = {1, 2};
    std::vector<int> basis_cstatus_out = {3, 4};

    void get_basis(int* rstatus, int* cstatus) const override
    {
        std::copy(basis_rstatus_out.begin(), basis_rstatus_out.end(), rstatus);
        std::copy(basis_cstatus_out.begin(), basis_cstatus_out.end(), cstatus);
    }

    std::vector<int> set_basis_rstatus;
    std::vector<int> set_basis_cstatus;
    int set_basis_calls = 0;

    void set_basis(std::span<int> rstatus, std::span<int> cstatus) override
    {
        set_basis_rstatus.assign(rstatus.begin(), rstatus.end());
        set_basis_cstatus.assign(cstatus.begin(), cstatus.end());
        ++set_basis_calls;
    }
};
