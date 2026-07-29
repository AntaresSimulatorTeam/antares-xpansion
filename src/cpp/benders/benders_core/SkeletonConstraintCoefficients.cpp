#include "antares-xpansion/benders/benders_core/SkeletonConstraintCoefficients.h"

#include "antares-xpansion/benders/benders_core/SkeletonSolverLoader.h"

SkeletonConstraintCoefficients::SkeletonConstraintCoefficients(
  const std::filesystem::path& input_root,
  Logger& logger,
  std::string solver_name,
  int log_level,
  ProblemsFormat format,
  mpi::communicator* world):
    input_root_(input_root),
    memoptim_utils_({})
{
    logger_ = logger;
    SkeletonSolverLoader loader(logger_);
    solver_ = loader.Load(input_root_ / "constraints" / "constraints.mps",
                          solver_name,
                          solver_log_manager_,
                          log_level,
                          format);
    read_coeffs_and_indices();
}

SkeletonConstraintCoefficients::SkeletonConstraintCoefficients(
  const std::filesystem::path& input_root,
  Logger& logger,
  std::shared_ptr<SolverAbstract> solver,
  mpi::communicator* world):
    input_root_(input_root),
    solver_(std::move(solver)),
    memoptim_utils_({})
{
    logger_ = logger;
    read_coeffs_and_indices();
}

void SkeletonConstraintCoefficients::read_coeffs_and_indices()
{
    auto dir = input_root_ / "constraints";
    coef_set_.Load(memoptim_utils_,
                   dir / "coef.csv",
                   dir / "coef_cols.csv",
                   dir / "coef_rows.csv",
                   solver_);
    rhs_set_.Load(memoptim_utils_, dir / "rhs.csv", std::nullopt, dir / "rhs_rows.csv", solver_);
}

int SkeletonConstraintCoefficients::GetConstraintsNumber()
{
    return coef_set_.Count();
}

std::shared_ptr<SolverAbstract> SkeletonConstraintCoefficients::GetSolver()
{
    return solver_;
}

std::shared_ptr<SolverAbstract> SkeletonConstraintCoefficients::ApplyConstraintSet(
  const std::string& constraints_name)
{
    solver_->chg_coefs(coef_set_.RowIndices(),
                       coef_set_.ColIndices(),
                       coef_set_.CoefficientsFor(constraints_name));
    solver_->chg_rhs_values(rhs_set_.RowIndices(), rhs_set_.CoefficientsFor(constraints_name));
    return solver_;
}
