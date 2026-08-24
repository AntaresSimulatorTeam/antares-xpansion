#pragma once

#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "antares-xpansion/benders/benders_core/SolverRowExtractor.h"
#include "antares-xpansion/benders/benders_core/SubproblemWorker.h"
#include "antares-xpansion/xpansion_interfaces/ILogger.h"

class SubproblemConstraintsManager;
typedef std::shared_ptr<SubproblemConstraintsManager> SubproblemConstraintsManagerPtr;

class SubproblemConstraintsManager
{
public:
    // CACHE_PROBLEMS < 2: dedicated MPS file per subproblem.
    static SubproblemConstraintsManagerPtr FromConstraintsFile(
      const std::filesystem::path& constraint_file_path,
      const std::string& solver_name,
      const SolverLogManager& solver_log_manager,
      Logger& logger,
      int log_level,
      ProblemsFormat format,
      const std::shared_ptr<SubproblemWorker>& subproblem_worker);

    // CACHE_PROBLEMS >= 2: shared skeleton solver, already mutated by
    // SkeletonConstraintSetLoader::LoadConstraintSet for this constraint set.
    static SubproblemConstraintsManagerPtr FromSharedSolver(
      std::shared_ptr<SolverAbstract> solver,
      const std::shared_ptr<SubproblemWorker>& subproblem_worker);

    SolverRepresentedRows AddRows(std::string& row_name);
    std::vector<double> GetSubSolution();
    int GetVariableIndexInSub(std::string variable_id);
    void DeleteAddedRows(int base_size);

    SubproblemConstraintsManager(std::shared_ptr<SolverAbstract> solver,
                                 const std::shared_ptr<SubproblemWorker>& subproblem_worker);

private:
    void add_rows_to_subproblem(SolverRepresentedRows& new_row);

    SolverRowExtractor row_extractor_;
    std::shared_ptr<SubproblemWorker> subproblem_worker_;
};

typedef std::map<std::string, SubproblemConstraintsManagerPtr> SubproblemConstraintsManagerPtrMap;
