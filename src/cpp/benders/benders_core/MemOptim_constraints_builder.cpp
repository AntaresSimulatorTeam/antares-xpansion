#include "antares-xpansion/benders/benders_core/MemOptim_constraints_builder.h"

#include <iostream>

MemOptimConstraintsBuilder::MemOptimConstraintsBuilder(const std::filesystem::path& inputRoot,
                                                       Logger& logger,
                                                       std::string solver_name,
                                                       int log_level,
                                                       ProblemsFormat format):
    inputRoot_(inputRoot)
{
    logger_ = logger;
    build_constraints_skeleton(solver_name, solver_log_manager_, log_level, format);
    read_coeffs_and_indices();
}

MemOptimConstraintsBuilder::MemOptimConstraintsBuilder(const std::filesystem::path& inputRoot,
                                                       Logger& logger,
                                                       std::shared_ptr<SolverAbstract> solver):
    inputRoot_(inputRoot),
    solver_(std::move(solver))
{
    logger_ = logger;
    read_coeffs_and_indices();
}

void MemOptimConstraintsBuilder::read_coeffs_and_indices()
{
    auto constraints_dir = inputRoot_ / "constraints";
    memoptim_utils::read_keyed_coeffs_csv(constraints_dir / "coef.csv", coeffs_);
    memoptim_utils::read_indices_csv(constraints_dir / "coef_cols.csv", constraints_col_indices_, true, solver_);
    memoptim_utils::read_indices_csv(constraints_dir / "coef_rows.csv", constraints_row_indices_, false, solver_);
    memoptim_utils::read_keyed_coeffs_csv(constraints_dir / "obj_coef.csv", obj_coeffs_);
    memoptim_utils::read_indices_csv(constraints_dir / "obj_cols.csv", obj_col_indices_, true, solver_);
    memoptim_utils::read_keyed_coeffs_csv(constraints_dir / "rhs.csv", rhs_);
    memoptim_utils::read_indices_csv(constraints_dir / "rhs_rows.csv", rhs_row_indices_, false, solver_);
}

void MemOptimConstraintsBuilder::build_constraints_skeleton(std::string solver_name,
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
    std::filesystem::path skeleton_constraints = inputRoot_ / "constraints" / "constraints.mps";

    benders_problem_provider_ = std::make_shared<BendersProblemFromFile>(skeleton_constraints);
    solver_IO_.configure(solver_name, format);

    benders_problem_provider_->provide_problem(solver_IO_, solver_);
}

int MemOptimConstraintsBuilder::get_constraints_number()
{
    return coeffs_.size();
}

std::shared_ptr<SolverAbstract> MemOptimConstraintsBuilder::create_constraints_reader(
  const std::string& constraints_name)
{
    auto& coeffs = coeffs_[constraints_name];
    auto& coeffs_obj = obj_coeffs_[constraints_name];
    auto& rhs_values = rhs_[constraints_name];

    solver_->chg_coefs(constraints_row_indices_, constraints_col_indices_, coeffs);
    solver_->chg_obj(obj_col_indices_, coeffs_obj);
    solver_->chg_rhs_values(rhs_row_indices_, rhs_values);

    return solver_;
}
