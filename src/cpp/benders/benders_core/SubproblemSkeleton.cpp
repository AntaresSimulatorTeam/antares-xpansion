#include "antares-xpansion/benders/benders_core/SubproblemSkeleton.h"

#include <algorithm>
#include <numeric>
#include <utility>

#include "antares-xpansion/benders/benders_core/skeleton_coefficient_reader.h"

namespace
{
std::vector<double> Weighted(const std::vector<double>& coefficients, double slave_weight)
{
    std::vector<double> weighted(coefficients.size());
    std::ranges::transform(coefficients,
                           weighted.begin(),
                           [slave_weight](double coefficient)
                           { return coefficient * slave_weight; });
    return weighted;
}
} // namespace

void SubproblemSkeleton::Load(const std::filesystem::path& sub_dir,
                              std::vector<std::string> sub_problem_names,
                              const std::shared_ptr<SolverAbstract>& solver,
                              Logger& logger,
                              AbortFunc abort_func)
{
    const auto ncols = solver->get_ncols();
    skeleton_obj_coeffs_.resize(ncols);
    solver->get_obj(skeleton_obj_coeffs_.data(), 0, ncols - 1);
    all_col_indices_.resize(ncols);
    std::iota(all_col_indices_.begin(), all_col_indices_.end(), 0);

    SkeletonCoefficientReader reader(std::move(sub_problem_names));

    coef_set_.Load(reader,
                   sub_dir / "coef.csv",
                   sub_dir / "coef_cols.csv",
                   sub_dir / "coef_rows.csv",
                   solver,
                   logger,
                   abort_func);
    obj_set_.Load(reader,
                  sub_dir / "obj_coef.csv",
                  sub_dir / "obj_cols.csv",
                  std::nullopt,
                  solver,
                  logger,
                  abort_func);
    rhs_set_.Load(reader,
                  sub_dir / "rhs.csv",
                  std::nullopt,
                  sub_dir / "rhs_rows.csv",
                  solver,
                  logger,
                  abort_func);
}

void SubproblemSkeleton::ApplyTo(SolverAbstract& solver,
                                 const std::string& sub_name,
                                 double slave_weight)
{
    solver.chg_obj(all_col_indices_, Weighted(skeleton_obj_coeffs_, slave_weight));

    solver.chg_coefs(coef_set_.RowIndices(),
                     coef_set_.ColIndices(),
                     coef_set_.CoefficientsFor(sub_name));
    solver.chg_obj(obj_set_.ColIndices(),
                   Weighted(obj_set_.CoefficientsFor(sub_name), slave_weight));
    solver.chg_rhs_values(rhs_set_.RowIndices(), rhs_set_.CoefficientsFor(sub_name));
}

int SubproblemSkeleton::SubproblemCountForThisRank() const
{
    return rhs_set_.Count();
}
