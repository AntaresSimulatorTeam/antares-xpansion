#include "antares-xpansion/benders/benders_core/SkeletonCoefficientSet.h"

#include <cstdlib>

#include "antares-xpansion/xpansion_interfaces/ILogger.h"

void SkeletonCoefficientSet::Load(SkeletonCoefficientReader& skeleton_coefficient_reader,
                                  const std::filesystem::path& coeffs_csv,
                                  std::optional<std::filesystem::path> col_indices_csv,
                                  std::optional<std::filesystem::path> row_indices_csv,
                                  const std::shared_ptr<SolverAbstract>& solver,
                                  Logger& logger,
                                  AbortFunc abort_func)
{
    abort_func_ = std::move(abort_func);
    logger_ = logger;
    skeleton_coefficient_reader.read_keyed_coeffs_csv(coeffs_csv, coeffs_);
    if (col_indices_csv)
    {
        skeleton_coefficient_reader.read_indices_csv(*col_indices_csv, col_indices_, true, solver);
    }
    if (row_indices_csv)
    {
        skeleton_coefficient_reader.read_indices_csv(*row_indices_csv, row_indices_, false, solver);
    }
}

std::vector<double>& SkeletonCoefficientSet::CoefficientsFor(const std::string& name)
{
    if (coeffs_.find(name) == coeffs_.end())
    {
        logger_->display_message("from SkeletonCoefficientSet couldn't find key");
        if (abort_func_)
        {
            abort_func_(EXIT_FAILURE);
        }
    }
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
