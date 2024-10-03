#include "solver_utils.h"

void solver_getrows(const SolverAbstract &solver_p, std::vector<int> &mstart_p,
                    std::vector<int> &mclind_p,
                    std::vector<double> &dmatval_p,
                    int first_p, int last_p)
{

    int nelems_returned = 0;
  solver_p.get_rows(mstart_p.data(), mclind_p.data(), dmatval_p.data(),
                    solver_p.get_nelems(), &nelems_returned, first_p, last_p);
}

void solver_get_obj_func_coeffs(const SolverAbstract &solver_p,
                                std::vector<double> &obj_p, int first_p, int last_p)
{
  solver_p.get_obj(obj_p.data(), first_p, last_p);
}

void solver_addcols(SolverAbstract &solver_p, std::vector<double> const &objx_p,
                    std::vector<int> const &mstart_p,
                    std::vector<int> const &mrwind_p,
                    std::vector<double> const &dmatval_p,
                    std::vector<double> const &bdl_p, std::vector<double> const &bdu_p,
                    std::vector<char> const &colTypes_p,
                    std::vector<std::string> const &colNames_p)
{
    assert(objx_p.size() != 0);
    assert((objx_p.size() == mstart_p.size()) || (mstart_p.size() == 0));
    assert(mrwind_p.size() == dmatval_p.size());

    int newCols = static_cast<int>(colTypes_p.size());
    int ncolInit = static_cast<int>(solver_p.get_ncols());
    int newnnz = static_cast<int>(dmatval_p.size());

    solver_p.add_cols(newCols, newnnz, objx_p.data(), mstart_p.data(),
                      mrwind_p.data(), dmatval_p.data(), bdl_p.data(), bdu_p.data());

    std::vector<int> newIndex(newCols);
    for (int i = 0; i < newCols; i++)
    {
        newIndex[i] = ncolInit + i;
    }

    solver_p.chg_col_type(newIndex, colTypes_p);

    if (colNames_p.size() > 0)
    {
        int ncolFinal = solver_p.get_ncols();
        for (int i = ncolInit; i < ncolFinal; i++)
        {
          solver_p.chg_col_name(i, colNames_p[i - ncolInit]);
        }
    }
}

void solver_addrows(SolverAbstract &solver_p, std::vector<char> const &qrtype_p,
                    std::vector<double> const &rhs_p,
                    std::vector<double> const &range_p,
                    std::vector<int> const &mstart_p,
                    std::vector<int> const &mclind_p,
                    std::vector<double> const &dmatval_p,
                    std::vector<std::string> const &names) {
  assert(qrtype_p.size() == rhs_p.size());
  assert((range_p.size() == 0) || (range_p.size() == qrtype_p.size()));
  assert(mclind_p.size() == dmatval_p.size());

  int nrows = static_cast<int>(rhs_p.size());

  solver_p.add_rows(nrows, static_cast<int>(dmatval_p.size()), qrtype_p.data(),
                    rhs_p.data(), range_p.data(), mstart_p.data(),
                    mclind_p.data(), dmatval_p.data(), names);
}

void solver_getlpsolution(SolverAbstract::Ptr const solver_p, std::vector<double> &x_p)
{
    solver_p->get_lp_sol(x_p.data(), NULL, NULL);
}

void solver_getlpdual(SolverAbstract::Ptr const solver_p, std::vector<double> &dual_p)
{
    solver_p->get_lp_sol(NULL, dual_p.data(), NULL);
}

void solver_getlpreducedcost(SolverAbstract::Ptr const solver_p, std::vector<double> &dj_p)
{
    solver_p->get_lp_sol(NULL, NULL, dj_p.data());
}

void solver_getrowtype(const SolverAbstract &solver_p,
                       std::vector<char> &qrtype_p, int first_p, int last_p)
{
    if (last_p >= first_p)
    {
        solver_p.get_row_type(qrtype_p.data(), first_p, last_p);
    }
}

void solver_getrhs(const SolverAbstract &solver_p, std::vector<double> &rhs_p,
                   int first_p, int last_p)
{
    if (last_p >= first_p)
    {
        solver_p.get_rhs(rhs_p.data(), first_p, last_p);
    }
}

void solver_getrhsrange(SolverAbstract::Ptr const solver_p, std::vector<double> &range_p,
                        int first_p, int last_p)
{
    if (last_p >= first_p)
    {
        solver_p->get_rhs_range(range_p.data(), first_p, last_p);
    }
}

void solver_getcolinfo(const SolverAbstract &solver_p,
                       std::vector<char> &coltype_p,
                       std::vector<double> &bdl_p, std::vector<double> &bdu_p,
                       int first_p, int last_p)
{
    solver_p.get_lb(bdl_p.data(), first_p, last_p);
    solver_p.get_ub(bdu_p.data(), first_p, last_p);
    solver_p.get_col_type(coltype_p.data(), first_p, last_p);
}

void solver_deactivaterows(SolverAbstract::Ptr solver_p, std::vector<int> const &mindex)
{
    for (auto const &index : mindex)
    {
        solver_p->del_rows(index, index);
    }
}

//@WARN Codes returned depend on the solver used.
void solver_getbasis(SolverAbstract::Ptr solver_p, std::vector<int> &rstatus_p,
                     std::vector<int> &cstatus_p)
{
    solver_p->get_basis(rstatus_p.data(), cstatus_p.data());
}

void solver_chgbounds(SolverAbstract::Ptr solver_p,
                      std::vector<int> const &mindex_p,
                      std::vector<char> const &qbtype_p,
                      std::vector<double> const &bnd_p)
{
    assert(mindex_p.size() == qbtype_p.size());
    assert(mindex_p.size() == bnd_p.size());

    solver_p->chg_bounds(mindex_p, qbtype_p, bnd_p);
}

void solver_rename_vars(SolverAbstract *outSolver_p,
                        const std::vector<std::string> &names_p) {

    if (const auto ncols = outSolver_p->get_ncols(); ncols == names_p.size()) {
        for (int i = 0; i < ncols; i++) {
          outSolver_p->chg_col_name(i, names_p[i]);
        }
    }
}
