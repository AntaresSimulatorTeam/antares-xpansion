#include "antares-xpansion/benders/benders_core/BendersProblemFromFile.h"

BendersProblemFromFile::BendersProblemFromFile(const std::filesystem::path& problem_file_path):
    problem_file_path(problem_file_path)
{
}

void BendersProblemFromFile::provide_problem(const SolverIO& solver_io, std::shared_ptr<SolverAbstract> solver) const
{
    solver_io.read(solver.get(), problem_file_path);
}

std::filesystem::path BendersProblemFromFile::provide_file_path() const
{
    return problem_file_path;
}
