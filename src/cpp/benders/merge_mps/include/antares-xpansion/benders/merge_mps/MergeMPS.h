#pragma once

#include "antares-xpansion/benders/benders_core/common.h"
#include "antares-xpansion/benders/factories/WriterFactories.h"
#include "antares-xpansion/helpers/solver_utils.h"

class AbstractMergeMPS
{
public:
    AbstractMergeMPS(MergeMPSOptions options,
                     Logger logger,
                     std::shared_ptr<Output::OutputWriter> writer);

    virtual ~AbstractMergeMPS() = default;

    void launch();

    MergeMPSOptions _options;
    Logger _logger;
    std::shared_ptr<Output::OutputWriter> _writer;

    SolverAbstract::Ptr _ptr_merged_solver;

protected:
    virtual double get_objective_weight(const int nb_subproblems, const std::string& name) const = 0;
    virtual void add_coupling_constraints(const CouplingMap& candidates) = 0;

    CouplingMap get_candidates(const CouplingMap& structure);
    void export_problem();
    bool solve(const int nb_threads = 16);
    void output_solution(const CouplingMap& structure,
                         const CouplingMap& candidates,
                         const bool is_sol_optimal);
};

class MergeMasterSubproblemMPS: public AbstractMergeMPS
{
public:
    MergeMasterSubproblemMPS(MergeMPSOptions options,
                             Logger logger,
                             std::shared_ptr<Output::OutputWriter> writer);

private:
    double get_objective_weight(const int nb_subproblems, const std::string& name) const override;
    void add_coupling_constraints(const CouplingMap& candidates) override;
};

using MergeMPS = MergeMasterSubproblemMPS;
