#include "antares-xpansion/benders/benders_core/MemOptim_subproblem_builder.h"

#include <antares-xpansion/benders/benders_core/SolverIO.h>
#include <iostream>

#include <boost/mpi.hpp>

#include "antares-xpansion/benders/benders_core/SubproblemWorker.h"

MemOptimSubProblemBuilder::MemOptimSubProblemBuilder(const std::filesystem::path& inputRoot,
                                                     Logger& logger,
                                                     std::string solver_name,
                                                     int log_level,
                                                     ProblemsFormat format):
    inputRoot_(inputRoot)
{
    logger_ = logger;
    build_sub_skeleton(solver_name, solver_log_manager_, log_level, format);
    read_coeffs_and_indices(CoeffType::constraints);
    read_coeffs_and_indices(CoeffType::objective);
    read_coeffs_and_indices(CoeffType::rhs);
    micro_iters_ = false;
    warm_start_ = false;
}

MemOptimSubProblemBuilder::MemOptimSubProblemBuilder(const std::filesystem::path& inputRoot,
                                                     Logger& logger,
                                                     std::shared_ptr<SolverAbstract> solver):
    inputRoot_(inputRoot),
    solver_(std::move(solver))
{
    logger_ = logger;
    read_coeffs_and_indices(CoeffType::constraints);
    read_coeffs_and_indices(CoeffType::objective);
    read_coeffs_and_indices(CoeffType::rhs);
    micro_iters_ = false;
    warm_start_ = false;
}

void MemOptimSubProblemBuilder::read_coeffs_and_indices(CoeffType coeff_type)
{
    auto sub_dir = inputRoot_ / "sub";
    switch (coeff_type)
    {
    case CoeffType::constraints:
        memoptim_utils::read_keyed_coeffs_csv(sub_dir / "coef.csv", coeffs_);
        memoptim_utils::read_indices_csv(sub_dir / "coef_cols.csv",
                                         constraints_col_indices_,
                                         true,
                                         solver_);
        memoptim_utils::read_indices_csv(sub_dir / "coef_rows.csv",
                                         constraints_row_indices_,
                                         false,
                                         solver_);
        break;
    case CoeffType::objective:
        memoptim_utils::read_keyed_coeffs_csv(sub_dir / "obj_coef.csv", obj_coeffs_);
        memoptim_utils::read_indices_csv(sub_dir / "obj_cols.csv", obj_col_indices_, true, solver_);
        break;
    case CoeffType::rhs:
        memoptim_utils::read_keyed_coeffs_csv(sub_dir / "rhs.csv", rhs_);
        memoptim_utils::read_indices_csv(sub_dir / "rhs_rows.csv",
                                         rhs_row_indices_,
                                         false,
                                         solver_);
        break;
    }
}

void MemOptimSubProblemBuilder::build_sub_skeleton(std::string solver_name,
                                                   const SolverLogManager& solver_log_manager,
                                                   int log_level,
                                                   ProblemsFormat format)
{
    SolverFactory solver_factory(logger_);
    solver_ = solver_factory.create_solver(solver_name,
                                           SOLVER_TYPE::CONTINUOUS,
                                           solver_log_manager);

    solver_->set_threads(1);
    solver_->set_output_log_level(log_level);
    std::filesystem::path skeleton_sub = inputRoot_ / "sub" / "sub.mps";

    benders_problem_provider_ = std::make_shared<BendersProblemFromFile>(skeleton_sub);
    solver_IO_.configure(solver_name, format);

    benders_problem_provider_->provide_problem(solver_IO_, solver_);
}

int MemOptimSubProblemBuilder::get_sub_number()
{
    return rhs_.size();
}

std::shared_ptr<SubproblemWorker> MemOptimSubProblemBuilder::create_sub_solver_abstract(
  std::string sub_name,
  VariableMap& variable_map,
  double cut_coefficient_tolerance,
  double slave_weight)
{
    auto& coeffs_sub = coeffs_[sub_name];
    auto& coeffs_obj = obj_coeffs_[sub_name];
    auto& rhs_values = rhs_[sub_name];

    solver_->chg_coefs(constraints_row_indices_, constraints_col_indices_, coeffs_sub);
    solver_->chg_obj(obj_col_indices_, coeffs_obj);
    solver_->chg_rhs_values(rhs_row_indices_, rhs_values);

    auto subproblem_worker = std::make_shared<SubproblemWorker>(variable_map,
                                                                solver_,
                                                                logger_,
                                                                slave_weight);

    return subproblem_worker;
}
