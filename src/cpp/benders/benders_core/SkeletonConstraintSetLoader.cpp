#include "antares-xpansion/benders/benders_core/SkeletonConstraintSetLoader.h"

#include "antares-xpansion/benders/benders_core/SkeletonSolverLoader.h"

SkeletonConstraintSetLoader::SkeletonConstraintSetLoader(
  const std::filesystem::path& input_root,
  Logger& logger,
  std::string solver_name,
  int log_level,
  ProblemsFormat format,
  std::vector<std::string>&& constraints_names,
  boost::mpi::communicator* world):
    logger_(logger),
    input_root_(input_root),
    skeleton_coefficient_reader_(std::move(constraints_names)),
    world_(world)
{
    SkeletonSolverLoader loader(logger_);
    solver_ = loader.Load(input_root_ / "constraints" / "constraints.mps",
                          solver_name,
                          solver_log_manager_,
                          log_level,
                          format);
    read_coeffs_and_indices();
}

SkeletonConstraintSetLoader::SkeletonConstraintSetLoader(const std::filesystem::path& input_root,
                                                         Logger& logger,
                                                         std::shared_ptr<SolverAbstract> solver,
                                                         std::vector<std::string>&& constraints_names):
    logger_(logger),
    input_root_(input_root),
    solver_(std::move(solver)),
    skeleton_coefficient_reader_(std::move(constraints_names))
{
    read_coeffs_and_indices();
}

void SkeletonConstraintSetLoader::read_coeffs_and_indices()
{
    auto dir = input_root_ / "constraints";
    coef_set_.Load(skeleton_coefficient_reader_,
                   dir / "coef.csv",
                   dir / "coef_cols.csv",
                   dir / "coef_rows.csv",
                   solver_,
                   world_);
    rhs_set_.Load(skeleton_coefficient_reader_,
                  dir / "rhs.csv",
                  std::nullopt,
                  dir / "rhs_rows.csv",
                  solver_,
                  world_);
}

int SkeletonConstraintSetLoader::GetConstraintsNumber()
{
    return coef_set_.Count();
}

std::shared_ptr<SolverAbstract> SkeletonConstraintSetLoader::GetSolver()
{
    return solver_;
}

std::shared_ptr<SolverAbstract> SkeletonConstraintSetLoader::LoadConstraintSet(
  const std::string& constraints_name)
{
    solver_->chg_coefs(coef_set_.RowIndices(),
                       coef_set_.ColIndices(),
                       coef_set_.CoefficientsFor(constraints_name));
    solver_->chg_rhs_values(rhs_set_.RowIndices(), rhs_set_.CoefficientsFor(constraints_name));
    return solver_;
}
