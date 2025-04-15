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

private:
    std::shared_ptr<Output::OutputWriter> writer_;

protected:
    [[nodiscard]] virtual double get_objective_weight(int nb_subproblems,
                                                      const std::string& name) const
      = 0;
    virtual void add_coupling_constraints(const CouplingMap& candidates) = 0;

    CouplingMap get_candidates(const CouplingMap& structure);
    void export_problem();
    bool solve(int nb_threads = 16);
    void output_solution(const CouplingMap& structure,
                         const CouplingMap& candidates,
                         bool is_sol_optimal);

    MergeMPSOptions options_;
    Logger logger_;

    SolverAbstract::Ptr ptr_merged_solver_;
};

class MergeMasterSubproblemMPS: public AbstractMergeMPS
{
public:
    using AbstractMergeMPS::AbstractMergeMPS;

private:
    [[nodiscard]] double get_objective_weight(int nb_subproblems,
                                              const std::string& name) const override;
    void add_coupling_constraints(const CouplingMap& candidates) override;
};

using MergeMPS = MergeMasterSubproblemMPS;
