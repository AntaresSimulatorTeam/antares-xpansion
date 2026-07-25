#pragma once

#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "antares-xpansion/benders/benders_core/SolverRowExtractor.h"
#include "antares-xpansion/benders/benders_core/SubproblemWorker.h"
#include "antares-xpansion/xpansion_interfaces/ILogger.h"

class SubproblemConstraintRepository;
typedef std::shared_ptr<SubproblemConstraintRepository> SubproblemConstraintRepositoryPtr;

class SubproblemConstraintRepository
{
public:
    // CACHE_PROBLEMS < 2: dedicated MPS file per subproblem.
    static SubproblemConstraintRepositoryPtr FromConstraintsFile(
      const std::filesystem::path& constraint_file_path,
      const std::string& solver_name,
      const SolverLogManager& solver_log_manager,
      Logger& logger,
      int log_level,
      ProblemsFormat format,
      const std::shared_ptr<SubproblemWorker>& subproblem_worker);

    // CACHE_PROBLEMS >= 2: shared skeleton solver, already mutated by
    // ConstraintSetRepository::ApplyConstraintSet for this constraint set.
    static SubproblemConstraintRepositoryPtr FromSharedSolver(
      std::shared_ptr<SolverAbstract> solver,
      const std::shared_ptr<SubproblemWorker>& subproblem_worker);

    SolverRepresentedRows AppendConstraint(std::string& row_name);
    void RemoveAppendedConstraints();

    const std::shared_ptr<SubproblemWorker>& worker() const;

private:
    SubproblemConstraintRepository(std::shared_ptr<SolverAbstract> solver,
                                 const std::shared_ptr<SubproblemWorker>& subproblem_worker);

    SolverRowExtractor row_extractor_;
    std::shared_ptr<SubproblemWorker> subproblem_worker_;
    int initial_sub_size_;
};

typedef std::map<std::string, SubproblemConstraintRepositoryPtr> SubproblemConstraintRepositoryPtrMap;
