#include "ortools_utils.h"

void ORTreadmps(SolverAbstract::Ptr solver_p, 
    std::string const & filename_p)
{
    solver_p->read_prob(filename_p.c_str(), "MPS");
}

void ORTwritemps(SolverAbstract::Ptr const solver_p, std::string const & filename_p)
{
    solver_p->write_prob(filename_p.c_str(), "MPS");
}

void ORTwritelp(SolverAbstract::Ptr const solver_p, std::string const & filename_p)
{
    solver_p->write_prob(filename_p.c_str(), "LP");
}


void ORTgetrows(SolverAbstract::Ptr const solver_p,
                std::vector<int> & mstart_p,
                std::vector<int> & mclind_p,
                std::vector<double> & dmatval_p,
                int first_p, int last_p)
{

    int nelems_returned = 0;
    solver_p->get_rows(mstart_p.data(), mclind_p.data(), dmatval_p.data(),
        solver_p->get_nelems(), &nelems_returned, 0, solver_p->get_nrows() - 1);
}



void ORTchgobj(SolverAbstract::Ptr solver_p, std::vector<int> const & mindex_p, 
    std::vector<double> const & obj_p)
{
    solver_p->chg_obj(mindex_p.size(), mindex_p.data(), obj_p.data());
}

void ORTgetobj(SolverAbstract::Ptr const solver_p, std::vector<double> & obj_p, 
    int first_p, int last_p)
{
    solver_p->get_obj(obj_p.data(), first_p, last_p);
}

void ORTaddcols(SolverAbstract::Ptr solver_p,
                std::vector<double> const & objx_p,
                std::vector<int> const & mstart_p,
                std::vector<int> const & mrwind_p,
                std::vector<double> const & dmatval_p,
                std::vector<double> const & bdl_p, std::vector<double> const & bdu_p,
                std::vector<char> const & colTypes_p,
                std::vector<std::string> & colNames_p)
{
	assert(objx_p.size() != 0);
    assert((objx_p.size() == mstart_p.size()) || (mstart_p.size() == 0));
    assert(mrwind_p.size() == dmatval_p.size());

    int newCols = colTypes_p.size();
    int ncolInit = solver_p->get_ncols();
    int newnnz = dmatval_p.size();

    solver_p->add_cols(newCols, newnnz, objx_p.data(), mstart_p.data(), 
        mrwind_p.data(), dmatval_p.data(), bdl_p.data(), bdu_p.data());

    std::vector<int> newIndex(newCols);
    for (int i = 0; i < newCols; i++) {
        newIndex[i] = ncolInit + i;
    }

    solver_p->chg_col_type(newCols, newIndex.data(), colTypes_p.data());

    if (colNames_p.size() > 0) {
        int ncolFinal = solver_p->get_ncols();
        for (int i = ncolInit; i < ncolFinal; i++) {
            solver_p->chg_col_name(i, colNames_p[i - ncolInit]);
        }
    }
}

void ORTaddrows(SolverAbstract::Ptr solver_p,
                std::vector<char> const &  qrtype_p,
                std::vector<double>  const & rhs_p,
                std::vector<double>  const & range_p,
                std::vector<int> const & mstart_p,
                std::vector<int> const & mclind_p,
                std::vector<double> const & dmatval_p)
{
	assert(qrtype_p.size() == rhs_p.size());
	assert((mstart_p.size() == 0 ) || (mstart_p.size() == qrtype_p.size()) );
	assert((range_p.size() == 0 ) || (range_p.size() == qrtype_p.size()) );
	assert(mclind_p.size() == dmatval_p.size());

    int nrows = rhs_p.size();

    solver_p->add_rows(nrows, dmatval_p.size(), qrtype_p.data(), 
        rhs_p.data(), range_p.data(), mstart_p.data(), mclind_p.data(), dmatval_p.data());
}

void ORTgetlpsolution(SolverAbstract::Ptr const solver_p, std::vector<double> & x_p)
{
    solver_p->get_lp_sol(x_p.data(), NULL, NULL);
}

void ORTgetlpdual(SolverAbstract::Ptr const solver_p, std::vector<double> & dual_p)
{
    solver_p->get_lp_sol(NULL, dual_p.data(), NULL);
}

void ORTgetlpreducedcost(SolverAbstract::Ptr const solver_p, std::vector<double> & dj_p)
{
    solver_p->get_lp_sol(NULL, NULL, dj_p.data());
}

void ORTgetrowtype(SolverAbstract::Ptr const solver_p, std::vector<char> & qrtype_p, 
    int first_p, int last_p)
{
    if (last_p >= first_p) {
        solver_p->get_row_type(qrtype_p.data(), first_p, last_p);
    }
}

void ORTgetrhs(SolverAbstract::Ptr const solver_p, std::vector<double> & rhs_p, 
    int first_p, int last_p)
{
    if (last_p >= first_p) {
        solver_p->get_rhs(rhs_p.data(), first_p, last_p);
    }
}

void ORTgetrhsrange(SolverAbstract::Ptr const solver_p, std::vector<double> &  range_p, 
    int first_p, int last_p)
{
    if (last_p >= first_p) {
        solver_p->get_rhs_range(range_p.data(), first_p, last_p);
    }
}

void ORTgetcolinfo(SolverAbstract::Ptr const solver_p,
                    std::vector<char> & coltype_p,
                    std::vector<double> & bdl_p, std::vector<double> & bdu_p,
                    int first_p, int last_p)
{
    solver_p->get_lb(bdl_p.data(), first_p, last_p);
    solver_p->get_ub(bdu_p.data(), first_p, last_p);
    solver_p->get_col_type(coltype_p.data(), first_p, last_p);
}



void ORTdeactivaterows(SolverAbstract::Ptr solver_p, std::vector<int> const & mindex)
{
    for (auto const& index : mindex) {
        solver_p->del_rows(index, index);
    }
}


//@WARN Codes returned depend on the solver used. 
void ORTgetbasis(SolverAbstract::Ptr solver_p, std::vector<int> & rstatus_p, 
    std::vector<int> & cstatus_p)
{
    solver_p->get_basis(rstatus_p.data(), cstatus_p.data());
}

void ORTchgbounds(SolverAbstract::Ptr solver_p,
                  std::vector<int> const & mindex_p,
                  std::vector<char> const & qbtype_p,
                  std::vector<double> const & bnd_p)
{
    assert(mindex_p.size() == qbtype_p.size());
    assert(mindex_p.size() == bnd_p.size());
    
    solver_p->chg_bounds(mindex_p.size(), mindex_p.data(), qbtype_p.data(), bnd_p.data());
}

void ORTcopyandrenamevars(SolverAbstract::Ptr outSolver_p, 
                          SolverAbstract::Ptr const inSolver_p, 
                          std::vector<std::string>& names_p, 
                          std::string const& solver_name){

    //outSolver_p.reset();

    //copy and rename columns
    /*std::vector<double> obj_l;
	ORTgetobj(inSolver_p, obj_l, 0, inSolver_p->get_ncols() - 1);
    std::vector<double> lb_l;
	std::vector<double> ub_l;
	std::vector<char> coltype_l;
	ORTgetcolinfo(inSolver_p, coltype_l, lb_l, ub_l, 0, inSolver_p->get_ncols() - 1);
	ORTaddcols(outSolver_p, obj_l, {}, {}, {}, lb_l, ub_l, coltype_l, names_p);

    //const std::vector<operations_research::MPVariable*> & outVariables_l = outSolver_p.variables();
    assert( inSolver_p->get_ncols() == outSolver_p->get_ncols() );

    //copy constraints
    for(auto inConstraint_l : inSolver_p.constraints())
    {
        operations_research::MPConstraint* outConstraint_l = outSolver_p.MakeRowConstraint(inConstraint_l->lb(), inConstraint_l->ub(), inConstraint_l->name());
        for(auto pairVarCoeff_l : inConstraint_l->terms())
        {
            outConstraint_l->SetCoefficient(outVariables_l[pairVarCoeff_l.first->index()], pairVarCoeff_l.second);
        }
    }*/

    //SolverFactory factory;
    //outSolver_p = factory.create_solver(solver_name, inSolver_p);

    for (int i = 0; i < outSolver_p->get_ncols() - 1; i++) {
        outSolver_p->chg_col_name(i, names_p[i]);
    }
}
