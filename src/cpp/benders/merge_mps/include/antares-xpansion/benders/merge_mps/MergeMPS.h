#pragma once

#include <optional>

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

    virtual void launch(bool export_pb = false, bool solve_pb = false) = 0;

private:
    std::shared_ptr<Output::OutputWriter> writer_;

protected:
    // [[nodiscard]] virtual double get_objective_weight(int nb_subproblems,
    //                                                   const std::string& name) const
    //   = 0;
    
    virtual void add_coupling_constraints() = 0;
    virtual void build_problem() = 0;

    void export_problem();

    [[nodiscard]] SolverAbstract::Ptr get_local_solver(const std::filesystem::path& root_dir,
                                                       const std::string& filename) const;
    void multiply_obj_by_weight_factor(SolverAbstract& local_solver, double weight) const;
    VariableMap merge_local_solver(SolverAbstract& local_solver,
                                   const std::string& local_prefix,
                                   const VariableMap& local_var_map,
                                   const std::string& filename);

    std::shared_ptr<Output::OutputWriter> writer_;
    MergeMPSOptions options_;
    Logger logger_;

    const SolverFactory factory_;
    SolverAbstract::Ptr ptr_merged_solver_;
};

class MergeMasterSubproblemMPS: public AbstractMergeMPS
{
public:
    using AbstractMergeMPS::AbstractMergeMPS;

    void launch() override;

private:
    void build_problem();
    bool solve(int nb_threads = 16);
    void output_solution(bool is_sol_optimal);

    [[nodiscard]] double get_problem_obj_weight(int nb_subproblems, const std::string& name) const;
    void add_coupling_constraints();

    CouplingMap structure_;
};

class MergeMasterMasterMPS: public AbstractMergeMPS
{
public:
    struct PathwayConstraints
    {
        double min_investment;
        double max_investment;

        double min_decommissioning;
        double max_decommissioning;
    };

    struct PathwayNode
    {
        PathwayNode(const std::string&& node, const Json::Value& data);

        std::string name;
        std::filesystem::path path{};

        std::optional<std::string> parent{std::nullopt};
        double weight{1.};

        VariableMap variables{};
        std::map<std::string, PathwayConstraints> constraints{};
    };

    using PathwayTree = std::vector<PathwayNode>;

    MergeMasterMasterMPS(MergeMPSOptions options,
                         Logger logger,
                         std::shared_ptr<Output::OutputWriter> writer,
                         const std::filesystem::path& tree_filename);

    void launch() override;

private:
    [[nodiscard]] double get_objective_weight(int nb_subproblems,
                                              const std::string& name) const;

    void build_problem() override;
    void add_coupling_constraints() override;
};

using MergeMPS = MergeMasterSubproblemMPS;
using MergePathwayMPS = MergeMasterMasterMPS;
