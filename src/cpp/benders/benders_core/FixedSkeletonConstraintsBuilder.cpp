#include "antares-xpansion/benders/benders_core/FixedSkeletonConstraintsBuilder.h"

FixedSkeletonConstraintsBuilder::FixedSkeletonConstraintsBuilder(
  const std::filesystem::path& inputRoot,
  Logger& logger,
  std::string solver_name,
  int log_level,
  ProblemsFormat format,
  mpi::communicator* world,
  const std::vector<std::string>& constraints_names):
    inputRoot_(inputRoot),
    memoptim_utils_(constraints_names)
{
    logger_ = logger;
    build(solver_name, solver_log_manager_, log_level, format);
    read_coeffs_and_indices();
}

FixedSkeletonConstraintsBuilder::FixedSkeletonConstraintsBuilder(
  const std::filesystem::path& inputRoot,
  Logger& logger,
  std::shared_ptr<SolverAbstract> solver,
  mpi::communicator* world):
    inputRoot_(inputRoot),
    solver_(std::move(solver)),
    memoptim_utils_({})
{
    logger_ = logger;
    read_coeffs_and_indices();
}

void FixedSkeletonConstraintsBuilder::read_coeffs_and_indices()
{
    auto constraints_dir = inputRoot_ / "constraints";

    memoptim_utils_.read_keyed_coeffs_csv(constraints_dir / "coef.csv", coeffs_);
    
    memoptim_utils_.read_indices_csv(constraints_dir / "coef_cols.csv",
                                     constraints_col_indices_,
                                     true,
                                     solver_);
    memoptim_utils_.read_indices_csv(constraints_dir / "coef_rows.csv",
                                     constraints_row_indices_,
                                     false,
                                     solver_);
    memoptim_utils_.read_keyed_coeffs_csv(constraints_dir / "rhs.csv", rhs_);
    
    memoptim_utils_.read_indices_csv(constraints_dir / "rhs_rows.csv",
                                     rhs_row_indices_,
                                     false,
                                     solver_);
}

void FixedSkeletonConstraintsBuilder::build(std::string solver_name,
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

int FixedSkeletonConstraintsBuilder::get_constraints_number()
{
    return coeffs_.size();
}

std::shared_ptr<SolverAbstract> FixedSkeletonConstraintsBuilder::update_constraints_reader(
  const std::string& constraints_name)
{
    auto& coeffs = coeffs_[constraints_name];
    auto& rhs_values = rhs_[constraints_name];

    solver_->chg_coefs(constraints_row_indices_, constraints_col_indices_, coeffs);
    solver_->chg_rhs_values(rhs_row_indices_, rhs_values);
    return solver_;
}

std::shared_ptr<SolverAbstract> FixedSkeletonConstraintsBuilder::get_solver() 
{
    return solver_ ; 
}

