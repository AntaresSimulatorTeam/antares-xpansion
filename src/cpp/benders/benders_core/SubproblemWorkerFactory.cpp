#include "antares-xpansion/benders/benders_core/SubproblemWorkerFactory.h"

#include <antares-xpansion/benders/benders_core/SolverIO.h>
#include <iostream>

#include "antares-xpansion/benders/benders_core/SkeletonSolverLoader.h"
#include "antares-xpansion/benders/benders_core/SubproblemWorker.h"

SubproblemWorkerFactory::SubproblemWorkerFactory(const std::filesystem::path& input_root,
                                                 Logger& logger,
                                                 std::string solver_name,
                                                 int log_level,
                                                 ProblemsFormat format,
                                                 std::vector<std::string> sub_problem_names):
    input_root_(input_root),
    memoptim_utils_(std::move(sub_problem_names))
{
    logger_ = logger;
    SkeletonSolverLoader loader(logger_);
    solver_ = loader.Load(input_root_ / "sub" / "sub.mps",
                          solver_name,
                          solver_log_manager_,
                          log_level,
                          format);
    load_coefficient_sets();
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

void SubproblemWorkerFactory::SetBasis(std::string sub_name)
{
    int row_number = solver_->get_nrows();
    int col_number = solver_->get_ncols();
    std::vector<int> rstatus(row_number);
    std::vector<int> cstatus(col_number);

    solver_->get_basis(rstatus.data(), cstatus.data());

    subpb_basis_[sub_name] = std::make_pair(std::move(rstatus), std::move(cstatus));
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

void SubproblemWorkerFactory::SetAddedConstraints(
  std::string sub_name,
  std::vector<SolverRepresentedRows>& added_constraints)
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
    solver_->chg_coefs(coef_set_.RowIndices(),
                       coef_set_.ColIndices(),
                       coef_set_.CoefficientsFor(sub_name));
    solver_->chg_obj(obj_set_.ColIndices(), obj_set_.CoefficientsFor(sub_name));
    solver_->chg_rhs_values(rhs_set_.RowIndices(), rhs_set_.CoefficientsFor(sub_name));

    // for warm start
    if (subpb_basis_[sub_name].first.size() > 0) [[likely]]
    {
        std::cout << "using warm start " << std::endl;
        solver_->set_basis(subpb_basis_[sub_name].first, subpb_basis_[sub_name].second);
    }

    auto subproblem_worker = std::make_shared<SubproblemWorker>(variable_map,
                                                                slave_weight,
                                                                solver_,
                                                                logger_);

    for (auto& solver_row: added_constraints_per_sub_[sub_name])
    {
        subproblem_worker->AddRows(solver_row.qrtype_p,
                                   solver_row.rhs,
                                   solver_row.range_p,
                                   solver_row.mstart,
                                   solver_row.mclind,
                                   solver_row.dmatval,
                                   solver_row.row_names);
    }

    return subproblem_worker;
}
