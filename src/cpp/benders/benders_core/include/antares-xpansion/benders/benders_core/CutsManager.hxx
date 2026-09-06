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

    void BuildAllAggregatedCuts(
      const std::vector<SubProblemNamesInCut>& subproblem_names,
      const std::vector<SubProblemDataMap>& gathered_subproblem_map,
      const VariableMap& problem_to_id,
      double& ub,
      const Point& x_cut,
      SubProblemDataMap& cut_trace,
      const WorkerMasterPtr& master)
    {
        std::vector<int> subproblem_ids_per_cut;
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
