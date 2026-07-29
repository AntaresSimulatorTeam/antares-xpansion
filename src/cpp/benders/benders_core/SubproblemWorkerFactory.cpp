#include "antares-xpansion/benders/benders_core/SubproblemWorkerFactory.h"

#include <antares-xpansion/benders/benders_core/SolverIO.h>
#include <iostream>

#include "antares-xpansion/benders/benders_core/SkeletonSolverLoader.h"
#include "antares-xpansion/benders/benders_core/SubproblemWorker.h"

SubproblemWorkerFactory::SubproblemWorkerFactory(
  const std::filesystem::path& input_root,
  Logger& logger,
  std::string solver_name,
  int log_level,
  ProblemsFormat format,
  std::vector<std::string> sub_problem_names,
  const SolverLogManager& solver_log_manager,
  std::shared_ptr<SolverAbstract> constraints_SolverAbstact):
    input_root_(input_root),
    memoptim_utils_(std::move(sub_problem_names)),
    constraintsSolverAbstract_(constraints_SolverAbstact)

{
    logger_ = logger;
    SkeletonSolverLoader loader(logger_);
    solver_ = loader.Load(input_root_ / "sub" / "sub.mps",
                          solver_name,
                          solver_log_manager,
                          log_level,
                          format);
    load_coefficient_sets();
    SubProblemSolverInitialSize_ = solver_->get_nrows();
}

SubproblemWorkerFactory::SubproblemWorkerFactory(const std::filesystem::path& input_root,
                                                 Logger& logger,
                                                 std::shared_ptr<SolverAbstract> solver,
                                                 std::vector<std::string> sub_problem_names):
    input_root_(input_root),
    solver_(std::move(solver)),
    memoptim_utils_(std::move(sub_problem_names))
{
    logger_ = logger;
    load_coefficient_sets();
}

void SubproblemWorkerFactory::load_coefficient_sets()
{
    auto dir = input_root_ / "sub";
    coef_set_.Load(memoptim_utils_,
                   dir / "coef.csv",
                   dir / "coef_cols.csv",
                   dir / "coef_rows.csv",
                   solver_);
    obj_set_.Load(memoptim_utils_,
                  dir / "obj_coef.csv",
                  dir / "obj_cols.csv",
                  std::nullopt,
                  solver_);
    rhs_set_.Load(memoptim_utils_, dir / "rhs.csv", std::nullopt, dir / "rhs_rows.csv", solver_);
}

void SubproblemWorkerFactory::SetAddedConstraints(std::string sub_name,
                                                  std::vector<std::string>& added_constraints)
{
    added_constraints_per_sub_[sub_name].insert(added_constraints_per_sub_[sub_name].end(),
                                                std::make_move_iterator(added_constraints.begin()),
                                                std::make_move_iterator(added_constraints.end()));
}

int SubproblemWorkerFactory::GetSubNumber()
{
    return rhs_set_.Count();
}

std::shared_ptr<SubproblemWorker> SubproblemWorkerFactory::CreateSubSolverAbstract(
  std::string sub_name,
  VariableMap& variable_map,
  double cut_coefficient_tolerance,
  double slave_weight)
{
    auto num_rows = solver_->get_nrows();
    if (num_rows != SubProblemSolverInitialSize_) [[likely]]
    {
        num_rows--;
        solver_->del_rows(SubProblemSolverInitialSize_, num_rows);
    }
    solver_->chg_coefs(coef_set_.RowIndices(),
                       coef_set_.ColIndices(),
                       coef_set_.CoefficientsFor(sub_name));
    solver_->chg_obj(obj_set_.ColIndices(), obj_set_.CoefficientsFor(sub_name));
    solver_->chg_rhs_values(rhs_set_.RowIndices(), rhs_set_.CoefficientsFor(sub_name));

    // for warm start
    if (basis_cache_.TryRestore(sub_name, *solver_))
    {
        std::cout << "using warm start " << std::endl;
    }

    auto subproblem_worker = std::make_shared<SubproblemWorker>(variable_map,
                                                                slave_weight,
                                                                solver_,
                                                                logger_);

    for (auto& solver_row_name: added_constraints_per_sub_[sub_name])
    {
        auto row_index = constraintsSolverAbstract_->get_row_index(solver_row_name);
        if (row_index < 0) [[inlikely]]
        {
            std::cerr << "can't find " << solver_row_name << " in constraints solver" << std::endl;
        }
        else
        {
            auto solver_row = SolverRowExtractor::GetRow(constraintsSolverAbstract_, row_index);
            subproblem_worker->AddRow(solver_row);
        }
    }

    return subproblem_worker;
}
