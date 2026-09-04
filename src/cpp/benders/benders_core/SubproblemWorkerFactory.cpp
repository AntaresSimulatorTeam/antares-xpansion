#include "antares-xpansion/benders/benders_core/SubproblemWorkerFactory.h"

#include <algorithm>
#include <numeric>

#include <antares-xpansion/benders/benders_core/SolverIO.h>
#include <iostream>

#include "antares-xpansion/benders/benders_core/SkeletonSolverLoader.h"
#include "antares-xpansion/benders/benders_core/SubproblemWorker.h"

SubproblemWorkerFactory::SubproblemWorkerFactory(const std::filesystem::path& input_root,
                                                 Logger& logger,
                                                 std::string solver_name,
                                                 int log_level,
                                                 ProblemsFormat format,
                                                 std::vector<std::string> sub_problem_names,
                                                 const SolverLogManager& solver_log_manager,
                                                 std::function<void()> fatal_error_handler):
    logger_(logger),
    input_root_(input_root),
    skeleton_coefficient_reader_(std::move(sub_problem_names)),
    fatal_error_handler_(std::move(fatal_error_handler))
{
    SkeletonSolverLoader loader(logger_);
    solver_ = loader.Load(input_root_ / "sub" / "sub.mps",
                          solver_name,
                          solver_log_manager,
                          log_level,
                          format);
    skeletonObjCoeffs_.resize(solver_->get_ncols());
    solver_->get_obj(skeletonObjCoeffs_.data(), 0, solver_->get_ncols() - 1);
    load_coefficient_sets();
    SubProblemSolverInitialSize_ = solver_->get_nrows();
}

void SubproblemWorkerFactory::GetBasis(std::string sub_name)
{
    subproblem_basis_cache_.Store(sub_name, *solver_);
}

SubproblemWorkerFactory::SubproblemWorkerFactory(const std::filesystem::path& input_root,
                                                 Logger& logger,
                                                 std::shared_ptr<SolverAbstract> solver,
                                                 std::vector<std::string> sub_problem_names):
    logger_(logger),
    input_root_(input_root),
    solver_(std::move(solver)),
    skeleton_coefficient_reader_(std::move(sub_problem_names))
{
    logger_ = logger;
    skeletonObjCoeffs_.resize(solver_->get_ncols());
    solver_->get_obj(skeletonObjCoeffs_.data(), 0, solver_->get_ncols() - 1);

    load_coefficient_sets();
}

std::shared_ptr<SolverAbstract> SubproblemWorkerFactory::GetSolver()
{
    return solver_;
}

void SubproblemWorkerFactory::load_coefficient_sets()
{
    auto dir = input_root_ / "sub";
    coef_set_.Load(skeleton_coefficient_reader_,
                   dir / "coef.csv",
                   dir / "coef_cols.csv",
                   dir / "coef_rows.csv",
                   solver_,
                   logger_,
                   fatal_error_handler_);
    obj_set_.Load(skeleton_coefficient_reader_,
                  dir / "obj_coef.csv",
                  dir / "obj_cols.csv",
                  std::nullopt,
                  solver_,
                  logger_,
                  fatal_error_handler_);
    rhs_set_.Load(skeleton_coefficient_reader_,
                  dir / "rhs.csv",
                  std::nullopt,
                  dir / "rhs_rows.csv",
                  solver_,
                  logger_,
                  fatal_error_handler_);
}

int SubproblemWorkerFactory::GetSubNumber()
{
    return rhs_set_.Count();
}

void SubproblemWorkerFactory::ApplyBasis(const std::string& sub_name)
{
    // Must be called once the solver's row/column structure has been reset to
    // sub_name's own shape (see Benders_MICRO_ITERS::OnBendersSubResolutionStart):
    // the solver instance can be shared/reused across subproblems (skeleton
    // mode), so applying a cached basis before that reset can mismatch a
    // stale row/column count left over from a different subproblem.
    subproblem_basis_cache_.TryRestore(sub_name, *solver_, logger_);
}

std::shared_ptr<SubproblemWorker> SubproblemWorkerFactory::CreateSubSolverAbstract(
  std::string sub_name,
  VariableMap& variable_map,
  double slave_weights)

{
    std::vector<double> weighted_obj(skeletonObjCoeffs_.size());
    std::ranges::transform(skeletonObjCoeffs_,
                           weighted_obj.begin(),
                           [slave_weights](double coefficient)
                           { return coefficient * slave_weights; });

    std::vector<int> indices(skeletonObjCoeffs_.size());
    std::iota(indices.begin(), indices.end(), 0);

    solver_->chg_obj(indices, weighted_obj);

    solver_->chg_coefs(coef_set_.RowIndices(),
                       coef_set_.ColIndices(),
                       coef_set_.CoefficientsFor(sub_name));

    const auto& obj_coeffs = obj_set_.CoefficientsFor(sub_name);
    std::vector<double> weighted_obj_coeffs(obj_coeffs.size());
    std::ranges::transform(obj_coeffs,
                           weighted_obj_coeffs.begin(),
                           [slave_weights](double coefficient)
                           { return coefficient * slave_weights; });
    solver_->chg_obj(obj_set_.ColIndices(), weighted_obj_coeffs);

    solver_->chg_rhs_values(rhs_set_.RowIndices(), rhs_set_.CoefficientsFor(sub_name));

    auto subproblem_worker = std::make_shared<SubproblemWorker>(variable_map, solver_, logger_);

    return subproblem_worker;
}
