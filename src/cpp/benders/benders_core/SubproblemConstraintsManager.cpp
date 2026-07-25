#include "antares-xpansion/benders/benders_core/SubproblemConstraintsManager.h"

#include "antares-xpansion/benders/benders_core/SkeletonSolverLoader.h"

SubproblemConstraintsManager::SubproblemConstraintsManager(
  std::shared_ptr<SolverAbstract> solver,
  const std::shared_ptr<SubproblemWorker>& subproblem_worker):
    row_extractor_(std::move(solver)),
    subproblem_worker_(subproblem_worker),
    initial_sub_size_(subproblem_worker->get_problem_row_num())
{
}

SubproblemConstraintsManagerPtr SubproblemConstraintsManager::FromConstraintsFile(
  const std::filesystem::path& constraint_file_path,
  const std::string& solver_name,
  const SolverLogManager& solver_log_manager,
  Logger& logger,
  int log_level,
  ProblemsFormat format,
  const std::shared_ptr<SubproblemWorker>& subproblem_worker)
{
    SkeletonSolverLoader loader(logger);
    auto solver = loader.Load(constraint_file_path,
                              solver_name,
                              solver_log_manager,
                              log_level,
                              format);
    return SubproblemConstraintsManagerPtr(
      new SubproblemConstraintsManager(std::move(solver), subproblem_worker));
}

SubproblemConstraintsManagerPtr SubproblemConstraintsManager::FromSharedSolver(
  std::shared_ptr<SolverAbstract> solver,
  const std::shared_ptr<SubproblemWorker>& subproblem_worker)
{
    return SubproblemConstraintsManagerPtr(
      new SubproblemConstraintsManager(std::move(solver), subproblem_worker));
}

const std::shared_ptr<SubproblemWorker>& SubproblemConstraintsManager::worker() const
{
    return subproblem_worker_;
}

SolverRepresentedRows SubproblemConstraintsManager::AddRows(std::string& row_name)
{
    auto constraint_row = row_extractor_.GetRow(row_name);
    subproblem_worker_->AddRow(constraint_row);
    return constraint_row;
}

void SubproblemConstraintsManager::DeleteAddedRows()
{
    subproblem_worker_->delete_rows(initial_sub_size_);
}
