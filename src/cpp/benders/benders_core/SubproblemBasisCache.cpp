#include "antares-xpansion/benders/benders_core/SubproblemBasisCache.h"

void SubproblemBasisCache::Store(const std::string& name, SolverAbstract& solver)
{
    int row_number = solver.get_nrows();
    int col_number = solver.get_ncols();
    std::vector<int> rstatus(row_number);
    std::vector<int> cstatus(col_number);

    solver.get_basis(rstatus.data(), cstatus.data());

    basis_per_name_[name] = std::make_pair(std::move(rstatus), std::move(cstatus));
}

bool SubproblemBasisCache::TryRestore(const std::string& name, SolverAbstract& solver)
{
    auto it = basis_per_name_.find(name);
    if (it == basis_per_name_.end())
    {
        return false;
    }
    solver.set_basis(it->second.first, it->second.second);
    return true;
}
