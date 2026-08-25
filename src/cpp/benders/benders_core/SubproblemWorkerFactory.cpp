#include "antares-xpansion/benders/benders_core/SubproblemWorkerFactory.h"

#include <antares-xpansion/benders/benders_core/SolverIO.h>

#include <iostream>

#include "antares-xpansion/benders/benders_core/SkeletonSolverLoader.h"

SubproblemWorkerFactory::SubproblemWorkerFactory(const std::filesystem::path& input_root,
                                                 Logger& logger,
                                                 std::string solver_name,
                                                 int log_level,
                                                 ProblemsFormat format,
                                                 std::vector<std::string> sub_problem_names,
                                                 const SolverLogManager& solver_log_manager,
                                                 boost::mpi::communicator* world):
    logger_(logger)
{
    SkeletonSolverLoader loader(logger_);
    solver_ = loader.Load(input_root / "sub" / "sub.mps",
                          solver_name,
                          solver_log_manager,
                          log_level,
                          format);
    skeleton_.Load(input_root / "sub", std::move(sub_problem_names), solver_, logger_, world);
}

SubproblemWorkerFactory::SubproblemWorkerFactory(const std::filesystem::path& input_root,
                                                 Logger& logger,
                                                 std::shared_ptr<SolverAbstract> solver,
                                                 std::vector<std::string> sub_problem_names):
    logger_(logger),
    solver_(std::move(solver))
{
    skeleton_.Load(input_root / "sub", std::move(sub_problem_names), solver_, logger_);
}

void SubproblemWorkerFactory::GetBasis(std::string sub_name)
{
    subproblem_basis_cache_.Store(sub_name, *solver_);
}

std::shared_ptr<SolverAbstract> SubproblemWorkerFactory::GetSolver()
{
    return solver_;
}

int SubproblemWorkerFactory::SubproblemCountForThisRank() const
{
    return skeleton_.SubproblemCountForThisRank();
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
  double slave_weight)
{
    skeleton_.ApplyTo(*solver_, sub_name, slave_weight);

    return std::make_shared<SubproblemWorker>(variable_map, solver_, logger_);
}
