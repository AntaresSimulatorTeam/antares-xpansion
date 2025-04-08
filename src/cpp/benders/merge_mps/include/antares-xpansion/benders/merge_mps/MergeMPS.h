#pragma once

#include "antares-xpansion/benders/benders_core/common.h"
#include "antares-xpansion/benders/factories/WriterFactories.h"
#include "antares-xpansion/helpers/solver_utils.h"

class MergeMPS
{
public:
    MergeMPS(MergeMPSOptions options, Logger logger, std::shared_ptr<Output::OutputWriter> writer);

    void launch();

    MergeMPSOptions _options;
    Logger _logger;
    std::shared_ptr<Output::OutputWriter> _writer;

private:
    CouplingMap get_candidates(const std::filesystem::path& root_dir,
                               const CouplingMap& structure,
                               const std::string& solver_to_use,
                               SolverAbstract::Ptr& ptr_merged_solver);

    double get_subproblem_weight(const int nb_subproblems, const std::string& name) const;
    void add_coupling_constraints(SolverAbstract& merged_solver, const CouplingMap& candidates);
    bool solve(SolverAbstract& merged_solver, const int nb_threads = 16);
    void output_solution(SolverAbstract& merged_solver,
                         const CouplingMap& structure,
                         const CouplingMap& candidates,
                         const bool is_sol_optimal);
};
