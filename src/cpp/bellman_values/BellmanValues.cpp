
#include "antares-xpansion/bellman_values/BellmanValues.h"

#include <ranges>

#include "antares-xpansion/grid_evaluator/Interpolator.h"

BellmanValues::BellmanValues(GridEvaluator& gridEvaluator,
                             const ReservoirManagement& reservoirManagement):
    gridEvaluator(gridEvaluator),
    reservoirManagement(reservoirManagement)
{
}

const std::vector<double>& BellmanValues::getLevels()
{
    return levels;
}

auto linspace(double start, double end, int num)
{
    if (num <= 0)
    {
        return std::vector<double>{};
    }
    if (num == 1)
    {
        return std::vector<double>{start};
    }

    double step = (end - start) / (num - 1);

    auto result = std::views::iota(0, num)
                  | std::views::transform([=](int i) { return start + i * step; });

    return std::vector<double>(result.begin(), result.end());
}

/// @brief Compute the Bellman values
/// @return The bellman values for each week
std::vector<std::vector<double>> BellmanValues::compute(int startWeek, int endWeek, int nbLevels)
{
    auto variationDeNiveauxDeStockData = gridEvaluator.ComputeRewards(startWeek, endWeek);

    levels = linspace(0.0, reservoirManagement.reservoir.capacity, nbLevels);
    std::map<ScenarioAndWeek, std::vector<double>> V;
    std::map<ScenarioAndWeek, std::vector<double>> rewards;

    for (const auto& [key, reward]: variationDeNiveauxDeStockData)
    {
        rewards[{key.scenario, key.week}].push_back(reward);
    }

    for (int week = startWeek; week <= endWeek + 1; ++week)
    {
        for (int scenario = 1; scenario <= gridEvaluator.nbScenarios; ++scenario)
        {
            V[{scenario, week}] = std::vector<double>(levels.size(), 0.0);
        }
    }

    auto gridDef = gridEvaluator.gridDefinition;
    for (int week = endWeek; week >= startWeek; --week)
    {
        for (int scenario = 1; scenario <= gridEvaluator.nbScenarios; ++scenario)
        {
            auto V_fut = [this, &V, &week, &scenario]()
            {
                auto& V_vec = V[{scenario, week + 1}];
                return [this, &V_vec, &week, &scenario](double x)
                { return Interpolator::linearInterpolation(this->levels, V_vec)(x); };
            };
            auto valuesVect = gridDef.weekAreaConstraints.at(week)
                                .at(gridDef.gridElements[0].area)
                                .at(gridDef.gridElements[0].name);
            for (size_t i = 0; i < levels.size(); ++i)
            {
                double Vu = solveWeeklyProblemWithReward(week,
                                                         endWeek,
                                                         scenario,
                                                         levels[i],
                                                         levels,
                                                         rewards[{scenario, week}],
                                                         V_fut());
                V[{scenario, week}][i] += Vu;
            }
        }

        for (int i = 0; i < levels.size(); ++i)
        {
            double sum = 0.0;
            for (int scenario = 1; scenario <= gridEvaluator.nbScenarios; ++scenario)
            {
                sum += V[{scenario, week}][i];
            }
            for (int scenario = 1; scenario <= gridEvaluator.nbScenarios; ++scenario)
            {
                V[{scenario, week}][i] = sum / gridEvaluator.nbScenarios;
            }
        }
    }

    std::vector<std::vector<double>> V_final;
    for (int week = startWeek; week <= endWeek; ++week)
    {
        V_final.push_back(V[{1, week}]);
    }

    return V_final;
}

/// @brief Solve the weekly problem
/// @param week
/// @param endWeek the last week of the problem
/// @param scenario
/// @param level the level of the reservoir
/// @param X the discretization of the reservoir level
/// @param rewards the rewards for each scenario and week
/// @param V_fut the reward function for the next week
/// @param breaking_points
/// @return The Bellvalue for a given week and scenario and level of the reservoir
double BellmanValues::solveWeeklyProblemWithReward(int week,
                                                   int endWeek,
                                                   int scenario,
                                                   double level,
                                                   const std::vector<double>& X,
                                                   const std::vector<double>& rewards,
                                                   const std::function<double(double)>& V_fut)
{
    double Vu = std::numeric_limits<double>::max();
    const Reservoir reservoir = reservoirManagement.reservoir;
    auto rewardFn = Interpolator::linearInterpolation(
      gridEvaluator.gridDefinition.gridElements[0].rhsValues[week - 1],
      rewards);
    auto penaltyFn = reservoirManagement.get_penalty(week, endWeek);

    for (double value_fut: X)
    {
        double u = -value_fut + level + reservoir.inflow[week - 1][scenario - 1];
        if ((-reservoir.max_pumping[week - 1] * reservoir.efficiency <= u)
            && (reservoirManagement.overflow || u <= reservoir.max_generating[week - 1]))
        {
            u = std::min(u, reservoir.max_generating[week - 1]);
            double G = rewardFn(u);
            double penalty = penaltyFn(value_fut);
            if (G + V_fut(value_fut) + penalty < Vu)
            {
                Vu = G + V_fut(value_fut) + penalty;
            }
        }
    }

    for (double u: gridEvaluator.gridDefinition.gridElements[0].rhsValues[week - 1])
    {
        double state_fut = level - u + reservoir.inflow[week - 1][scenario - 1];
        if (0 <= state_fut && state_fut <= reservoir.capacity)
        {
            double G = rewardFn(u);
            double penalty = penaltyFn(state_fut);
            if (G + V_fut(state_fut) + penalty < Vu)
            {
                Vu = G + V_fut(state_fut) + penalty;
            }
        }
    }

    if (week == endWeek && reservoirManagement.force_final_level)
    {
        double uFinal = level + reservoir.inflow[week - 1][scenario - 1]
                        - reservoirManagement.final_level;
        if (-reservoir.max_pumping[week - 1] * reservoir.efficiency <= uFinal
            && uFinal <= reservoir.max_generating[week - 1])
        {
            double state_fut = level - uFinal + reservoir.inflow[week - 1][scenario - 1];
            double penalty = penaltyFn(state_fut);
            if (rewardFn(uFinal) + V_fut(state_fut) + penalty < Vu)
            {
                Vu = rewardFn(uFinal) + V_fut(state_fut) + penalty;
            }
        }
    }
    else
    {
        double uMin = level + reservoir.inflow[week - 1][scenario - 1]
                      - reservoir.bottom_rule_curve[week - 1];
        if (-reservoir.max_pumping[week - 1] * reservoir.efficiency <= uMin
            && uMin <= reservoir.max_generating[week - 1])
        {
            double state_fut = level - uMin + reservoir.inflow[week - 1][scenario - 1];
            double penalty = penaltyFn(state_fut);
            if (rewardFn(uMin) + V_fut(state_fut) + penalty < Vu)
            {
                Vu = rewardFn(uMin) + V_fut(state_fut) + penalty;
            }
        }

        double uMax = level + reservoir.inflow[week - 1][scenario - 1]
                      - reservoir.upper_rule_curve[week - 1];
        if (-reservoir.max_pumping[week - 1] * reservoir.efficiency <= uMax
            && uMax <= reservoir.max_generating[week - 1])
        {
            double state_fut = level - uMax + reservoir.inflow[week - 1][scenario - 1];
            double penalty = penaltyFn(state_fut);
            if (rewardFn(uMax) + V_fut(state_fut) + penalty < Vu)
            {
                Vu = rewardFn(uMax) + V_fut(state_fut) + penalty;
            }
        }
    }

    return Vu;
}
