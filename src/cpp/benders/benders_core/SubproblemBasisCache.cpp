#include "antares-xpansion/benders/benders_core/SubproblemBasisCache.h"

#include <iostream>

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
    // A cached basis can outgrow the subproblem's structural size (e.g. micro-iterations
    // add rows mid-solve before the basis is stored), while the solver instance it is
    // restored onto later is freshly built at the subproblem's original size. Applying a
    // basis sized for a larger problem then writes past the solver's row/column status
    // arrays, so any size mismatch must be treated as a cache miss instead.
    // Fundamentally, a cache miss indicates a bug in the code handling the warm start and should be
    // investigated by developers. The fallback to no warm start is kept to avoid a crash at the
    // cost a longer simulation time, but without impact on the final result
    if (static_cast<int>(it->second.first.size()) != solver.get_nrows()
        || static_cast<int>(it->second.second.size()) != solver.get_ncols())
    {
        std::cerr
          << "Warning: cached basis for subproblem '" << name
          << "' does not match the solver's current size (" << it->second.first.size() << " vs "
          << solver.get_nrows() << " rows, " << it->second.second.size() << " vs "
          << solver.get_ncols()
          << " cols) - skipping basis restore, warm start will not work for this subproblem."
          << std::endl;
        return false;
    }
    solver.set_basis(it->second.first, it->second.second);
    return true;
}
