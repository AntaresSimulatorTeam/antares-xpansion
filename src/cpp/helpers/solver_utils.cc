#include "antares-xpansion/helpers/solver_utils.h"

void solver_getrows(const SolverAbstract& solver_p,
                    std::vector<int>& mstart_p,
                    std::vector<int>& mclind_p,
                    std::vector<double>& dmatval_p,
                    int first_p,
                    int last_p)
{
    int nelems_returned = 0;
    solver_p.get_rows(mstart_p.data(),
                      mclind_p.data(),
                      dmatval_p.data(),
                      solver_p.get_nelems(),
                      &nelems_returned,
                      first_p,
                      last_p);
}

void solver_get_obj_func_coeffs(const SolverAbstract& solver_p,
                                std::vector<double>& obj_p,
                                int first_p,
                                int last_p)
{
    solver_p.get_obj(obj_p.data(), first_p, last_p);
}

void solver_addcols(SolverAbstract& solver_p,
                    const std::vector<double>& objx_p,
                    const std::vector<int>& mstart_p,
                    const std::vector<int>& mrwind_p,
                    const std::vector<double>& dmatval_p,
                    const std::vector<double>& bdl_p,
                    const std::vector<double>& bdu_p,
                    const std::vector<char>& colTypes_p,
                    const std::vector<std::string>& colNames_p)
{
    assert(objx_p.size() != 0);
    assert((objx_p.size() == mstart_p.size()) || (mstart_p.size() == 0));
    assert(mrwind_p.size() == dmatval_p.size());

    int newCols = static_cast<int>(colTypes_p.size());
    int ncolInit = static_cast<int>(solver_p.get_ncols());
    int newnnz = static_cast<int>(dmatval_p.size());

    solver_p.add_cols(newCols,
                      newnnz,
                      objx_p.data(),
                      mstart_p.data(),
                      mrwind_p.data(),
                      dmatval_p.data(),
                      bdl_p.data(),
                      bdu_p.data(),
                      {});

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

void solver_addrows(SolverAbstract& solver_p,
                    const std::vector<char>& qrtype_p,
                    const std::vector<double>& rhs_p,
                    const std::vector<double>& range_p,
                    const std::vector<int>& mstart_p,
                    const std::vector<int>& mclind_p,
                    const std::vector<double>& dmatval_p,
                    const std::vector<std::string>& names)
{
    assert(qrtype_p.size() == rhs_p.size());
    assert((range_p.size() == 0) || (range_p.size() == qrtype_p.size()));
    assert(mclind_p.size() == dmatval_p.size());

    int nrows = static_cast<int>(rhs_p.size());

    solver_p.add_rows(nrows,
                      static_cast<int>(dmatval_p.size()),
                      qrtype_p.data(),
                      rhs_p.data(),
                      range_p.data(),
                      mstart_p.data(),
                      mclind_p.data(),
                      dmatval_p.data(),
                      names);
}

void solver_getlpsolution(std::shared_ptr<SolverAbstract> solver_p, std::vector<double>& x_p)
{
    solver_p->get_lp_sol(x_p.data(), NULL, NULL);
}

void solver_getlpdual(std::shared_ptr<SolverAbstract> solver_p, std::vector<double>& dual_p)
{
    solver_p->get_lp_sol(NULL, dual_p.data(), NULL);
}

void solver_getlpreducedcost(std::shared_ptr<SolverAbstract> solver_p, std::vector<double>& dj_p)
{
    solver_p->get_lp_sol(NULL, NULL, dj_p.data());
}

void solver_getrowtype(const SolverAbstract& solver_p,
                       std::vector<char>& qrtype_p,
                       int first_p,
                       int last_p)
{
    if (last_p >= first_p)
    {
        solver_p.get_row_type(qrtype_p.data(), first_p, last_p);
    }
}

void solver_getrhs(const SolverAbstract& solver_p,
                   std::vector<double>& rhs_p,
                   int first_p,
                   int last_p)
{
    if (last_p >= first_p)
    {
        solver_p.get_rhs(rhs_p.data(), first_p, last_p);
    }
}

void solver_getrhsrange(std::shared_ptr<SolverAbstract> solver_p,
                        std::vector<double>& range_p,
                        int first_p,
                        int last_p)
{
    if (last_p >= first_p)
    {
        solver_p->get_rhs_range(range_p.data(), first_p, last_p);
    }
}

void solver_getcolinfo(const SolverAbstract& solver_p,
                       std::vector<char>& coltype_p,
                       std::vector<double>& bdl_p,
                       std::vector<double>& bdu_p,
                       int first_p,
                       int last_p)
{
    solver_p.get_lb(bdl_p.data(), first_p, last_p);
    solver_p.get_ub(bdu_p.data(), first_p, last_p);
    solver_p.get_col_type(coltype_p.data(), first_p, last_p);
}

void solver_deactivaterows(std::shared_ptr<SolverAbstract> solver_p, const std::vector<int>& mindex)
{
    for (const auto& index: mindex)
    {
        solver_p->del_rows(index, index);
    }
}

//@WARN Codes returned depend on the solver used.
void solver_getbasis(std::shared_ptr<SolverAbstract> solver_p,
                     std::vector<int>& rstatus_p,
                     std::vector<int>& cstatus_p)
{
    solver_p->get_basis(rstatus_p.data(), cstatus_p.data());
}

void solver_chgbounds(std::shared_ptr<SolverAbstract> solver_p,
                      const std::vector<int>& mindex_p,
                      const std::vector<char>& qbtype_p,
                      const std::vector<double>& bnd_p)
{
    assert(mindex_p.size() == qbtype_p.size());
    assert(mindex_p.size() == bnd_p.size());

    solver_p->chg_bounds(mindex_p, qbtype_p, bnd_p);
}

void solver_chg_rhs(std::shared_ptr<SolverAbstract> solver_p, int id_row, double val)
{
    assert(id_row >= 0);
    assert(id_row < solver_p->get_nrows());
    solver_p->chg_rhs(id_row, val);
}

void solver_rename_vars(SolverAbstract* outSolver_p, const std::vector<std::string>& names_p)
{
    if (const auto ncols = outSolver_p->get_ncols(); ncols == names_p.size())
    {
        for (int i = 0; i < ncols; i++)
        {
            outSolver_p->chg_col_name(i, names_p[i]);
        }
    }
}
