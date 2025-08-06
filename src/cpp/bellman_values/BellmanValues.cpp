
#include "antares-xpansion/bellman_values/BellmanValues.h"

#include <ranges>

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

template<typename XContainer, typename YContainer>
double interpolate(const XContainer& X, const YContainer& Y, double x_query)
{
    if (X.size() != Y.size() || X.empty())
    {
        throw std::invalid_argument("X and Y must have the same size and not be empty");
    }

    auto x_begin = X.begin();
    auto x_end = X.end();

    // Clamp below range
    if (x_query <= *x_begin)
    {
        return *Y.begin();
    }

    // Clamp above range
    auto last = std::prev(x_end);
    if (x_query >= *last)
    {
        return *std::prev(Y.end());
    }

    // Find first element >= x_query
    auto it_upper = std::lower_bound(x_begin, x_end, x_query);

    if (it_upper != x_end && *it_upper == x_query)
    {
        // Exact match
        auto y_it = Y.begin();
        std::advance(y_it, std::distance(x_begin, it_upper));
        return *y_it;
    }

    // Otherwise interpolate between previous and it_upper
    auto it_lower = std::prev(it_upper);

    double x0 = *it_lower;
    double x1 = *it_upper;

    auto y_it_lower = Y.begin();
    std::advance(y_it_lower, std::distance(x_begin, it_lower));

    auto y_it_upper = Y.begin();
    std::advance(y_it_upper, std::distance(x_begin, it_upper));

    double y0 = *y_it_lower;
    double y1 = *y_it_upper;

    return y0 + (y1 - y0) * (x_query - x0) / (x1 - x0);
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

    for (int week = endWeek; week >= startWeek; --week)
    {
        for (int scenario = 1; scenario <= gridEvaluator.nbScenarios; ++scenario)
        {
            auto V_fut = [this, &V, &week, &scenario]()
            {
                auto& V_vec = V[{scenario, week + 1}];
                return [this, &V_vec, &week, &scenario](double x)
                { return interpolate(this->levels, V_vec, x); };
            };

            std::set<double> breaking_points = {
              *gridEvaluator.gridDefinition.gridElements[0].values[{scenario, week}].begin(),
              *gridEvaluator.gridDefinition.gridElements[0].values[{scenario, week}].rbegin()};
            for (size_t i = 0; i < levels.size(); ++i)
            {
                double Vu = solveWeeklyProblemWithReward(week,
                                                         scenario,
                                                         levels[i],
                                                         levels,
                                                         rewards[{scenario, week}],
                                                         V_fut(),
                                                         breaking_points);
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
/// @param scenario
/// @param level the level of the reservoir
/// @param X the discretization of the reservoir level
/// @param rewards the rewards for each scenario and week
/// @param V_fut the reward function for the next week
/// @param breaking_points
/// @return The Bellvalue for a given week and scenario and level of the reservoir
double BellmanValues::solveWeeklyProblemWithReward(int week,
                                                   int scenario,
                                                   double level,
                                                   const std::vector<double>& X,
                                                   const std::vector<double>& rewards,
                                                   const std::function<double(double)>& V_fut,
                                                   std::set<double>& breaking_points)
{
    double Vu = std::numeric_limits<double>::max();
    const Reservoir reservoir = reservoirManagement.reservoir;

    for (double value_fut: X)
    {
        double u = -value_fut + level + reservoir.inflow[week - 1][scenario - 1];
        if ((-reservoir.max_pumping[week - 1] * reservoir.efficiency <= u)
            && (reservoirManagement.overflow || u <= reservoir.max_generating[week - 1]))
        {
            u = std::min(u, reservoir.max_generating[week - 1]);
            double G = interpolate(
              gridEvaluator.gridDefinition.gridElements[0].values[{scenario, week}],
              rewards,
              u);
            if (G + V_fut(value_fut) < Vu)
            {
                Vu = G + V_fut(value_fut);
            }
        }
    }

    for (double u: gridEvaluator.gridDefinition.gridElements[0].values[{scenario, week}])
    {
        double state_fut = level - u + reservoir.inflow[week - 1][scenario - 1];
        if (0 <= state_fut && state_fut <= reservoir.capacity)
        {
            double G = interpolate(
              gridEvaluator.gridDefinition.gridElements[0].values[{scenario, week}],
              rewards,
              u);
            if (G + V_fut(state_fut) < Vu)
            {
                Vu = G + V_fut(state_fut);
            }
        }
    }

    return Vu;
}
