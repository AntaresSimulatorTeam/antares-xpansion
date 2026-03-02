#include "antares-xpansion/benders/benders_core/SolverIO.h"
#include "antares-xpansion/multisolver_interface/Solver.h"

#pragma once

class IBendersProblemProvider
{
public:
    virtual ~IBendersProblemProvider() = default;
    virtual void provide_problem(const SolverIO& solver_io,
                                 std::shared_ptr<SolverAbstract> solver) const
      = 0;
    virtual std::filesystem::path provide_file_path() const = 0;
};
