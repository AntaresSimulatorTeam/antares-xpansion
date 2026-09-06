#pragma once

#include <vector>

#include "BendersStructsDatas.h"
#include "SubproblemCut.h"
#include "WorkerMaster.h"
#include "common.h"

namespace
{

inline void compute_cut_val(const Point& var_name_subgradient, const Point& x_cut, Point& s)
{
    for (const auto& [cand_name, cand_value]: x_cut)
    {
        const auto cand_name_and_subgradient = var_name_subgradient.find(cand_name);
        if (cand_name_and_subgradient != var_name_subgradient.end())
        {
            s[cand_name] += cand_name_and_subgradient->second;
        }
    }
}

} // namespace

template<typename Derived>
class CutsManager
{
public:
    CutsManager() = default;

    template<typename... Args>
    void GatherAndBuildCuts(Args&&... args)
    {
        static_cast<Derived*>(this)->GatherAndBuildCutsImpl(std::forward<Args>(args)...);
    }

    // Aggregates subproblem costs and tracks min/max simplex iterations
    // across all gathered subproblem data. Called during cut building to
    // populate iteration-level statistics before constructing the cuts.
    void SetSubproblemDataCostAndSimplexIter(
      const std::vector<SubProblemDataMap>& gathered_subproblem_map,
      CurrentIterationData& data)
    {
        for (const auto& subproblem_data_map: gathered_subproblem_map)
        {
            for (auto&& [sub_problem_name, subproblem_data]: subproblem_data_map)
            {
                data.subproblem_cost += subproblem_data.subproblem_cost;
                BoundSimplexIterations(subproblem_data.simplex_iter, data);
            }
        }
    }

    // Adds one individual cut per subproblem to the master problem.
    // Used when NB_CUTS_PER_ITER is 0 (no aggregation).
    void ComputeCut(const SubProblemDataMap& subproblem_data_map,
                    double& ub,
                    const Point& x_cut,
                    VariableMap& problem_to_id,
                    SubProblemDataMap& cut_trace,
                    const WorkerMasterPtr& master)
    {
        for (const auto& [subproblem_name, subproblem_data]: subproblem_data_map)
        {
            ub += subproblem_data.subproblem_cost;

            master->addSubproblemCut(problem_to_id[subproblem_name],
                                     subproblem_data.var_name_and_subgradient,
                                     x_cut,
                                     subproblem_data.subproblem_cost);

            cut_trace[subproblem_name] = subproblem_data;
        }
    }

    // Adds a single aggregated cut (summing all subproblem subgradients)
    // to the master problem. Used when NB_CUTS_PER_ITER > 0.
    void ComputeCutAggregate(const SubProblemDataMap& subproblem_data_map,
                             double& ub,
                             const Point& x_cut,
                             SubProblemDataMap& cut_trace,
                             const WorkerMasterPtr& master)
    {
        Point s;
        double rhs(0);
        for (const auto& [name, subproblem_data]: subproblem_data_map)
        {
            ub += subproblem_data.subproblem_cost;
            rhs += subproblem_data.subproblem_cost;

            compute_cut_val(subproblem_data.var_name_and_subgradient, x_cut, s);

            cut_trace[name] = subproblem_data;
        }
        master->add_cut(s, x_cut, rhs);
    }

    // Computes the separation point x_cut from x_out and x_in using the
    // separation parameter, then rounds values near variable bounds.
    void ComputeXCut(CurrentIterationData& data,
                     double separation_param,
                     double master_solution_tolerance)
    {
        if (data.it == 1)
        {
            data.x_in = data.x_out;
            data.x_cut = data.x_out;
            data.master_only_vars_in = data.master_only_vars_out;
            data.master_only_vars_cut = data.master_only_vars_out;
        }
        else
        {
            for (const auto& [name, value]: data.x_out)
            {
                data.x_cut[name] = separation_param * data.x_out[name]
                                   + (1 - separation_param) * data.x_in[name];
            }
            for (int i(0); i < data.master_only_vars_out.size(); ++i)
            {
                data.master_only_vars_cut[i] = separation_param * data.master_only_vars_out[i]
                                               + (1 - separation_param)
                                                   * data.master_only_vars_in[i];
            }
        }
        RoundXCut(data, master_solution_tolerance);
    }

    void BuildAllAggregatedCuts(const std::vector<SubProblemNamesInCut>& subproblem_names,
                                const std::vector<SubProblemDataMap>& gathered_subproblem_map,
                                const VariableMap& problem_to_id,
                                double& ub,
                                const Point& x_cut,
                                SubProblemDataMap& cut_trace,
                                const WorkerMasterPtr& master)
    {
        for (const auto& subproblem_names_in_cut: subproblem_names)
        {
            Point s;
            double rhs{0};
            std::vector<int> subproblem_ids_per_cut;

            for (const auto& [sub_problem_name, position_in_gathered]: subproblem_names_in_cut)
            {
                subproblem_ids_per_cut.push_back(problem_to_id.at(sub_problem_name));

                auto subproblem_data_pair = gathered_subproblem_map[position_in_gathered].find(
                  sub_problem_name);

                if (subproblem_data_pair != gathered_subproblem_map[position_in_gathered].end())
                {
                    auto& subproblem_data = subproblem_data_pair->second;
                    ub += subproblem_data.subproblem_cost;
                    rhs += subproblem_data.subproblem_cost;
                    compute_cut_val(subproblem_data.var_name_and_subgradient, x_cut, s);
                    cut_trace[sub_problem_name] = subproblem_data;
                }
            }

            master->addGroupSubproblemCut(subproblem_ids_per_cut, s, x_cut, rhs);
        }
    }

private:
    // Rounds x_cut values that are within tolerance of variable bounds to
    // avoid numerical drift from repeated separation parameter application.
    void RoundXCut(CurrentIterationData& data, double master_solution_tolerance)
    {
        for (auto& kvp: data.x_cut)
        {
            double value = kvp.second;
            double lb = data.min_invest.at(kvp.first);
            double ub = data.max_invest.at(kvp.first);

            if (std::abs(value - lb) < master_solution_tolerance)
            {
                kvp.second = lb;
            }
            else if (std::abs(value - ub) < master_solution_tolerance)
            {
                kvp.second = ub;
            }
        }
    }

    // Updates min/max simplex iteration bounds for the current iteration.
    // Used by SetSubproblemDataCostAndSimplexIter to track solver effort
    // across subproblems during cut gathering.
    void BoundSimplexIterations(int subproblem_iterations, CurrentIterationData& data)
    {
        data.max_simplexiter = (data.max_simplexiter < subproblem_iterations)
                                 ? subproblem_iterations
                                 : data.max_simplexiter;
        data.min_simplexiter = (data.min_simplexiter > subproblem_iterations)
                                 ? subproblem_iterations
                                 : data.min_simplexiter;
    }
};
