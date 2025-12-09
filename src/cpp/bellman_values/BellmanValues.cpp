
#include "antares-xpansion/bellman_values/BellmanValues.h"

#include <ranges>
#include <tuple>

#include <antares/api/solver.h>

#include "antares-xpansion/grid_evaluator/Interpolator.h"

BellmanValues::BellmanValues(GridEvaluator& gridEvaluator,
                             const ReservoirManagement& reservoirManagement):
    gridEvaluator(gridEvaluator),
    reservoirManagement(reservoirManagement)
{
}

/// @brief Get the levels of the reservoir.
/// @return The levels of the reservoir.
const std::vector<double>& BellmanValues::getLevels() const
{
    return levels;
}

/// @brief Generate a linearly spaced vector of doubles.
/// @param start The starting value of the sequence.
/// @param end The ending value of the sequence.
/// @param num The number of values to generate.
/// @return A vector of doubles with linearly spaced values.
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

template<typename T>
concept WeeklyProblemMap = requires { typename T::key_type; }
                           && std::same_as<typename T::key_type, Antares::Solver::WeeklyProblemId>;

/// @brief Get the years of the weekly problems
/// @param container The container of weekly problems
/// @return The set of years
template<WeeklyProblemMap MapType>
std::set<unsigned int> getYears(const MapType& container)
{
    auto years_view = container | std::views::keys // take only the keys
                      | std::views::transform([](const auto& id) { return id.year; });

    return std::set<unsigned int>(years_view.begin(), years_view.end());
}

/// @brief Get the weeks of the weekly problems
/// @param container The container of weekly problems
/// @return The set of weeks
template<WeeklyProblemMap MapType>
std::set<unsigned int> getWeeks(const MapType& container)
{
    auto weeks_view = container | std::views::keys
                      | std::views::transform([](const auto& id) { return id.week; });

    return std::set<unsigned int>(weeks_view.begin(), weeks_view.end());
}

/// @brief Compute the Bellman values
/// @param nbLevels The levels discretization's number
/// @return The bellman values for each week
std::vector<std::vector<double>> BellmanValues::compute(int nbLevels)
{
    auto variationDeNiveauxDeStockData = gridEvaluator.ComputeCostsAndDuals();

    levels = linspace(0.0, reservoirManagement.reservoir.capacity, nbLevels);
    std::map<Antares::Solver::WeeklyProblemId, std::vector<double>> V;
    std::map<Antares::Solver::WeeklyProblemId, std::vector<double>> costs;

    for (const auto& [key, data]: variationDeNiveauxDeStockData)
    {
        costs[{key.scenario - 1, key.week - 1}].push_back(data.cost);
    }

    auto scenarios = getYears(costs);
    auto weeks = getWeeks(costs);
    unsigned int startWeek = *weeks.begin();
    unsigned int endWeek = *weeks.rbegin();

    for (unsigned int scenario: scenarios)
    {
        for (unsigned int week = startWeek; week <= endWeek + 1; ++week)
        {
            V[{scenario, week}] = std::vector<double>(levels.size(), 0.0);
        }

        auto penalty_fn = reservoirManagement.get_penalty(endWeek, endWeek);
        for (int i_level = 0; i_level < levels.size(); ++i_level)
        {
            V[{scenario, endWeek + 1}][i_level] += penalty_fn(levels[i_level]);
        }
    }

    const auto& gridDef = gridEvaluator.gridDefinition;
    for (unsigned int week = endWeek + 1; week-- > startWeek;)
    {
        for (unsigned int scenario: scenarios)
        {
            auto V_fut = [this, &V, &week, &scenario]()
            {
                auto& V_vec = V[{scenario, week + 1}];
                return [this, &V_vec, &week, &scenario](double x)
                { return Interpolator::linearInterpolation(this->levels, V_vec)(x); };
            };
            auto valuesVect = gridDef.weekAreaConstraints.at(week + 1)
                                .at(gridDef.gridElements[0].area)
                                .at(gridDef.gridElements[0].name);
            for (size_t i = 0; i < levels.size(); ++i)
            {
                double Vu;
                std::tie(Vu, std::ignore, std::ignore) = solveWeeklyProblemWithCost(
                  week,
                  endWeek,
                  scenario,
                  levels[i],
                  levels,
                  costs[{scenario, week}],
                  V_fut());
                V[{scenario, week}][i] += Vu;
            }
        }

        for (int i = 0; i < levels.size(); ++i)
        {
            double sum = 0.0;
            for (unsigned int scenario: scenarios)
            {
                sum += V[{scenario, week}][i];
            }
            for (unsigned int scenario: scenarios)
            {
                V[{scenario, week}][i] = sum / scenarios.size();
            }
        }
    }

    std::vector<std::vector<double>> V_final;
    for (unsigned int week = startWeek; week <= endWeek + 1; ++week)
    {
        V_final.push_back(V[{*scenarios.begin(), week}]);
    }

    this->bellmanValues = V; // copy stored in case of multistock
    this->costs = costs;     // copy stored in case of multistock
    return V_final;
}

/// @brief Solve the weekly problem
/// @param week the current week
/// @param endWeek the last week of the problem
/// @param scenario the current scenario
/// @param level the level of the reservoir
/// @param X the discretization of the reservoir level
/// @param costs the costs for each scenario and week
/// @param V_fut the cost function for the next week
/// @return a tuple including 1) the Bellman value for a given week and scenario and level of the
/// reservoir, 2) the final level of stock, 3) the optimal control
std::tuple<double, double, double> BellmanValues::solveWeeklyProblemWithCost(
  int week,
  int endWeek,
  int scenario,
  double level,
  const std::vector<double>& X,
  const std::vector<double>& costs,
  const std::function<double(double)>& V_fut)
{
    double Vu = std::numeric_limits<double>::max(); // optimal objective value (Bellman value)
    double xf = 0.;                                 // final level of stock
    const Reservoir reservoir = reservoirManagement.reservoir;
    auto costFn = Interpolator::linearInterpolation(
      gridEvaluator.gridDefinition.gridElements[0].rhsValues[week],
      costs);
    auto penalty = reservoirManagement.get_penalty(week, endWeek + 1)(level);

    for (double value_fut: X)
    {
        double u = -value_fut + level + reservoir.inflow[week][scenario];
        if ((-reservoir.max_pumping[week] * reservoir.efficiency <= u)
            && (u <= reservoir.max_generating[week] + reservoir.inflow[week][scenario]))
        {
            u = std::min(u, reservoir.max_generating[week] + reservoir.inflow[week][scenario]);
            double G = costFn(u);
            if (G + V_fut(value_fut) + penalty < Vu)
            {
                Vu = G + V_fut(value_fut) + penalty;
                xf = value_fut;
            }
        }
    }

    for (double u: gridEvaluator.gridDefinition.gridElements[0].rhsValues[week])
    {
        double state_fut = level - u + reservoir.inflow[week][scenario];
        if (0 <= state_fut && state_fut <= reservoir.capacity)
        {
            double G = costFn(u);
            if (G + V_fut(state_fut) + penalty < Vu)
            {
                Vu = G + V_fut(state_fut) + penalty;
                xf = state_fut;
            }
        }
    }

    if (week == endWeek && reservoirManagement.force_final_level)
    {
        double uFinal = level + reservoir.inflow[week][scenario] - reservoirManagement.final_level;
        if (-reservoir.max_pumping[week] * reservoir.efficiency <= uFinal
            && uFinal <= reservoir.max_generating[week] + reservoir.inflow[week][scenario])
        {
            double state_fut = level - uFinal + reservoir.inflow[week][scenario];
            if (costFn(uFinal) + V_fut(state_fut) + penalty < Vu)
            {
                Vu = costFn(uFinal) + V_fut(state_fut) + penalty;
                xf = state_fut;
            }
        }
    }
    else
    {
        double uMin = level + reservoir.inflow[week][scenario] - reservoir.bottom_rule_curve[week];
        if (-reservoir.max_pumping[week] * reservoir.efficiency <= uMin
            && uMin <= reservoir.max_generating[week] + reservoir.inflow[week][scenario])
        {
            double state_fut = level - uMin + reservoir.inflow[week][scenario];
            if (costFn(uMin) + V_fut(state_fut) + penalty < Vu)
            {
                Vu = costFn(uMin) + V_fut(state_fut) + penalty;
                xf = state_fut;
            }
        }

        double uMax = level + reservoir.inflow[week][scenario] - reservoir.upper_rule_curve[week];
        if (-reservoir.max_pumping[week] * reservoir.efficiency <= uMax
            && uMax <= reservoir.max_generating[week] + reservoir.inflow[week][scenario])
        {
            double state_fut = level - uMax + reservoir.inflow[week][scenario];
            if (costFn(uMax) + V_fut(state_fut) + penalty < Vu)
            {
                Vu = costFn(uMax) + V_fut(state_fut) + penalty;
                xf = state_fut;
            }
        }
    }
    double control = -(xf - level - reservoir.inflow[week][scenario]); // optimal control

    return std::tie(Vu, xf, control);
}

std::vector<std::vector<double>> BellmanValues::computeOptimalTrajectories()
{
    auto scenarios = getYears(costs);
    auto weeks = getWeeks(costs);
    unsigned int startWeek = *weeks.begin();
    unsigned int endWeek = *weeks.rbegin();

    // initialization
    std::vector<std::vector<double>> trajectory = reservoirManagement.reservoir.optimal_trajectory;

    // for (unsigned int week = endWeek + 1; week-- > startWeek;)
    for (unsigned int week = startWeek; week < endWeek + 1; ++week)
    {
        std::cout << "Optimal trajectory, week " << week << std::endl;
        for (unsigned int scenario: scenarios)
        {
            std::cout << "Scenario " << scenario << std::endl;

            // future costs now coming from previously computed Bellman values
            auto V_fut = [this, &week, &scenario]()
            {
                auto& V_vec = this->bellmanValues[{scenario, week + 1}];
                return [this, &V_vec, &week, &scenario](double x)
                { return Interpolator::linearInterpolation(this->levels, V_vec)(x); };
            };

            std::cout << "V_fut OK" << std::endl;

            // the considered level is the trajectory of the previous week
            // or the initial level for the first week
            double level = week == 0
                             ? reservoirManagement.reservoir.initial_level
                             : reservoirManagement.reservoir.optimal_trajectory[week - 1][scenario];

            std::tie(std::ignore, trajectory[week][scenario], std::ignore)
              = solveWeeklyProblemWithCost(week,
                                           endWeek,
                                           scenario,
                                           level, // here: trajectory of the previous week
                                                  // (first week: initial level)
                                           levels,
                                           costs[{scenario, week}],
                                           V_fut());
        }
    }
    return trajectory;
}
