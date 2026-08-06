#include "antares-xpansion/benders/benders_core/SolverRowExtractor.h"

SolverRowExtractor::SolverRowExtractor(std::shared_ptr<SolverAbstract> solver):
    solver_(std::move(solver))
{
}

int SolverRowExtractor::get_row_index(const std::string& name)
{
    return solver_->get_row_index(name);
}

SolverRepresentedRows SolverRowExtractor::GetRow(std::shared_ptr<SolverAbstract> solver,
                                                 int row_pos)
{
    SolverRepresentedRows result;
    int ncols = solver->get_ncols();
    result.mstart.resize(2);
    result.mclind.resize(ncols);
    result.dmatval.resize(ncols);

    int nels(0);
    solver->get_rows(result.mstart.data(),
                     result.mclind.data(),
                     result.dmatval.data(),
                     ncols,
                     &nels,
                     row_pos,
                     row_pos);
    result.mclind.resize(nels);
    result.dmatval.resize(nels);
    result.mstart.resize(1);

    double rhs(0.);
    solver->get_rhs(&rhs, row_pos, row_pos);
    result.rhs = {rhs};

    const int MAX_LEN = 10;
    char buffer[MAX_LEN];
    solver->get_row_type(buffer, row_pos, row_pos);
    int len = 0;
    while (len < MAX_LEN && buffer[len] >= 'A' && buffer[len] <= 'Z')
    {
        ++len;
    }
    std::string qrtype(buffer, len);
    result.qrtype_p = {qrtype[0]};

    return result;
}

SolverRepresentedRows SolverRowExtractor::GetRow(const std::string& name)
{
    int constraint_pos = get_row_index(name);
    auto result = GetRow(solver_, constraint_pos);
    result.row_names = {name};
    return result;
}
