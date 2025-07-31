#include "IBendersProblemProvider.h"
#include "SolverIO.h"
#include "antares-xpansion/multisolver_interface/Solver.h"

#pragma once

class BendersProblemFromFile: public IBendersProblemProvider
{
public:
    explicit BendersProblemFromFile(const std::filesystem::path& problem_file_path);
    void provide_problem(const SolverIO& solver_io,
                         std::shared_ptr<SolverAbstract> solver) const override;
    std::filesystem::path provide_file_path() const override;
    const std::filesystem::path problem_file_path;
};
