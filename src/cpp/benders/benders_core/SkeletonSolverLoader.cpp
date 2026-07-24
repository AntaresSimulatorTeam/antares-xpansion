#include "antares-xpansion/benders/benders_core/SkeletonSolverLoader.h"

SkeletonSolverLoader::SkeletonSolverLoader(Logger& logger):
    logger_(logger)
{
}

std::shared_ptr<SolverAbstract> SkeletonSolverLoader::Load(
  const std::filesystem::path& mps_path,
  const std::string& solver_name,
  const SolverLogManager& solver_log_manager,
  int log_level,
  ProblemsFormat format)
{
    SolverIO solver_IO;
    solver_IO.configure(solver_name, format);

    SolverFactory solver_factory(logger_);
    auto solver = solver_factory.create_solver(solver_name,
                                               SOLVER_TYPE::CONTINUOUS,
                                               solver_log_manager);

    solver->set_threads(1);
    solver->set_output_log_level(log_level);

    auto benders_problem_provider = std::make_shared<BendersProblemFromFile>(mps_path);
    benders_problem_provider->provide_problem(solver_IO, solver);

    return solver;
}
