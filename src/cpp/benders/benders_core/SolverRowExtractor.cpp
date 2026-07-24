#include "antares-xpansion/benders/benders_core/SolverRowExtractor.h"

SolverRowExtractor::SolverRowExtractor(std::shared_ptr<SolverAbstract> solver):
    solver_(std::move(solver))
{
}

int SolverRowExtractor::get_row_index(const std::string& name)
{
    return solver_->get_row_index(name);
}

SolverRepresentedRows SolverRowExtractor::GetRow(const std::string& name)
{
    SolverRepresentedRows result;
    result.range_p = {};
    result.row_names = {name};
    int constraint_pos = get_row_index(name);

    int ncols = solver_->get_ncols();
    result.mstart.resize(2);
    result.mclind.resize(ncols);
    result.dmatval.resize(ncols);

    int nels(0);
    solver_->get_rows(result.mstart.data(),
                      result.mclind.data(),
                      result.dmatval.data(),
                      ncols,
                      &nels,
                      constraint_pos,
                      constraint_pos);

    result.mclind.resize(nels);
    result.dmatval.resize(nels);
    result.mstart.resize(1);

    double rhs(0.);
    solver_->get_rhs(&rhs, constraint_pos, constraint_pos);
    result.rhs = {rhs};

    double range_p(0.);
    solver_->get_rhs_range(&range_p, constraint_pos, constraint_pos);
    result.range_p = {range_p};

    const int MAX_LEN = 10;
    char buffer[MAX_LEN];
    solver_->get_row_type(buffer, constraint_pos, constraint_pos);
    int len = 0;
    while (len < MAX_LEN && buffer[len] >= 'A' && buffer[len] <= 'Z')
    {
        ++len;
    }
    std::string qrtype(buffer, len);
    result.qrtype_p = {qrtype[0]};

    return result;
}
