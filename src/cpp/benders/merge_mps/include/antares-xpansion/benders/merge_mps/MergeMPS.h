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

    void launch(bool export_pb = false, bool solve_pb = false);

private:
    std::shared_ptr<Output::OutputWriter> writer_;

protected:
    // [[nodiscard]] virtual double get_objective_weight(int nb_subproblems,
    //                                                   const std::string& name) const
    //   = 0;
    
    virtual void add_coupling_constraints() = 0;
    virtual void build_problem() = 0;

    void export_problem();
    bool solve(int nb_threads = 16);
    void output_solution(bool is_sol_optimal);

    void multiply_obj_by_weight_factor(SolverAbstract& local_solver, double weight) const;

    MergeMPSOptions options_;
    Logger logger_;

    SolverAbstract::Ptr ptr_merged_solver_;
    CouplingMap structure_;
};

class MergeMasterSubproblemMPS: public AbstractMergeMPS
{
public:
    using AbstractMergeMPS::AbstractMergeMPS;

private:
    [[nodiscard]] double get_objective_weight(int nb_subproblems,
                                              const std::string& name) const;

    void build_problem() override;
    void add_coupling_constraints() override;
};

using MergeMPS = MergeMasterSubproblemMPS;
