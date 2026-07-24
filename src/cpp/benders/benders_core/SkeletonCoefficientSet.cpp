#include "antares-xpansion/benders/benders_core/SkeletonCoefficientSet.h"

void SkeletonCoefficientSet::Load(MemoptimUtils& memoptim_utils,
                                  const std::filesystem::path& coeffs_csv,
                                  std::optional<std::filesystem::path> col_indices_csv,
                                  std::optional<std::filesystem::path> row_indices_csv,
                                  const std::shared_ptr<SolverAbstract>& solver)
{
    memoptim_utils.read_keyed_coeffs_csv(coeffs_csv, coeffs_);
    if (col_indices_csv)
    {
        memoptim_utils.read_indices_csv(*col_indices_csv, col_indices_, true, solver);
    }
    if (row_indices_csv)
    {
        memoptim_utils.read_indices_csv(*row_indices_csv, row_indices_, false, solver);
    }
}

std::vector<double>& SkeletonCoefficientSet::CoefficientsFor(const std::string& name)
{
    return coeffs_[name];
}

std::vector<int>& SkeletonCoefficientSet::RowIndices()
{
    return row_indices_;
}

std::vector<int>& SkeletonCoefficientSet::ColIndices()
{
    return col_indices_;
}

int SkeletonCoefficientSet::Count() const
{
    return coeffs_.size();
}
