#pragma once

#include <optional>

#include "antares-xpansion/benders/benders_core/SolverIO.h"
#include "antares-xpansion/benders/benders_core/common.h"
#include "antares-xpansion/benders/factories/WriterFactories.h"
#include "antares-xpansion/helpers/solver_utils.h"

constexpr char MERGE_MPS_LOGGER_CONTEXT[] = "MergeMPS";

class AbstractMergeMPS
{
public:
    AbstractMergeMPS(MergeMPSOptions options,
                     Logger logger,
                     std::shared_ptr<Output::OutputWriter> writer);

    virtual ~AbstractMergeMPS() = default;

    virtual void launch() = 0;

protected:
    void terminate(const std::string& location, const std::string& message) const;

    void export_problem(const std::string& filename = "log_merged", bool export_lp = false) const;

    [[nodiscard]] std::shared_ptr<SolverAbstract> get_local_solver(
      const std::filesystem::path& root_dir,
      const std::string& filename) const;
    void multiply_obj_by_weight_factor(SolverAbstract& local_solver, double weight) const;
    VariableMap merge_local_solver(SolverAbstract& local_solver,
                                   const std::string& local_prefix,
                                   const VariableMap& local_var_map,
                                   const std::string& filename) const;

    std::shared_ptr<Output::OutputWriter> writer_;
    MergeMPSOptions options_;
    Logger logger_;

    const SolverFactory factory_;
    SolverIO solver_io_;
    std::shared_ptr<SolverAbstract> ptr_merged_solver_;
};

class MergeMasterSubproblemMPS: public AbstractMergeMPS
{
public:
    using AbstractMergeMPS::AbstractMergeMPS;

    void launch() override;

private:
    void build_problem();
    bool solve(int nb_threads = 16) const;
    void output_solution(bool is_sol_optimal);

    [[nodiscard]] double get_problem_obj_weight(int nb_subproblems, const std::string& name) const;
    void add_coupling_constraints();

    CouplingMap structure_;
};

using MergeMPS = MergeMasterSubproblemMPS;
