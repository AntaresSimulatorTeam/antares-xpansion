
#include "antares-xpansion/bellman_values/BellmanValues.h"

#include <limits>
#include <numeric>
#include <ranges>
#include <tuple>

#include <antares/api/solver.h>

#include "antares-xpansion/evaluator/Interpolator.h"

BellmanValues::BellmanValues(GridEvaluator& gridEvaluator,
                             const ReservoirManagement& reservoirManagement,
                             Logger logger):
    gridEvaluator(gridEvaluator),
    reservoirManagement(reservoirManagement),
    logger{std::move(logger)}
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
/// @return The bellman values and the costs for each week
std::pair<std::vector<std::vector<double>>, std::vector<std::vector<double>>>
BellmanValues::compute(int nbLevels)
{
    auto variationDeNiveauxDeStockData = gridEvaluator.ComputeCostsAndDuals();

    logger->display_message("Computed costs and duals", LogUtils::LOGLEVEL::INFO, logger->CONTEXT);

    levels = linspace(0.0, reservoirManagement.reservoir.capacity, nbLevels);
    std::map<Antares::Solver::WeeklyProblemId, std::vector<double>> V;
    std::map<Antares::Solver::WeeklyProblemId, std::vector<double>> costs;

    for (const auto& [key, data]: variationDeNiveauxDeStockData)
    {
        costs[{key.scenario - 1, key.week - 1}].push_back(data.subproblem_cost);
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

            for (size_t i = 0; i < levels.size(); ++i)
            {
                double Vu;
                double control;
                std::tie(Vu,
                         std::ignore,
                         control) = solveWeeklyProblemWithCost(week,
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
            if (reservoirManagement.cvar == 1.0)
            {
                // no sorting of scenarios by cost necessary (default case)
                double sum = 0.0;
                for (unsigned int scenario: scenarios)
                {
                    sum += V[{scenario, week}][i];
                }
                double average = sum / scenarios.size();
                for (unsigned int scenario: scenarios)
                {
                    V[{scenario, week}][i] = average;
                }
            }
            else if (reservoirManagement.cvar == 0.0)
            {
                // only the most expensive scenario is taken into account
                double max = std::numeric_limits<double>::min();
                for (unsigned int scenario: scenarios)
                {
                    max = std::max(V[{scenario, week}][i], max);
                }
                for (unsigned int scenario: scenarios)
                {
                    V[{scenario, week}][i] = max;
                }
            }
            else
            {
                // CVaR has been requested, scenarios must be sorted by cost, then only the most
                // 1/CVaR expensive scenarios will be taken into account.
                // given the data structure, copying data is simpler, and takes only a small amount
                // of time in tested cases
                std::vector<double> values;
                values.reserve(scenarios.size());
                for (unsigned int scenario: scenarios)
                {
                    values.push_back(V[{scenario, week}][i]);
                }
                // pivot, with rounding to nearest index
                auto pivot = values.end()
                             - static_cast<int>((values.size() - 1) * reservoirManagement.cvar + 1);
                // sort highest values to the right of the pivot
                std::nth_element(values.begin(), pivot, values.end());
                // sum
                double sum = std::accumulate(pivot, values.end(), 0.0);
                // number of elements for averaging
                int nb_elem = std::distance(pivot, values.end());
                double average = sum / nb_elem;

                for (unsigned int scenario: scenarios)
                {
                    V[{scenario, week}][i] = average;
                }
            }
        }
    }

    std::vector<std::vector<double>> V_final;
    std::vector<std::vector<double>> costs_final;

    for (unsigned int week = startWeek; week <= endWeek; ++week)
    {
        V_final.push_back(V.at({*scenarios.begin(), week}));
        costs_final.push_back(costs.at({*scenarios.begin(), week}));
    }
    // extra last week in V_final:
    V_final.push_back(V.at({*scenarios.begin(), endWeek + 1}));

    this->bellmanValues = V; // copy stored in case of multistock
    this->costs = costs;     // copy stored in case of multistock
    return std::make_pair(V_final, costs_final);
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
    std::vector<DblVector> rhsValues = gridEvaluator.gridDefinition.getRhsValuesForWeek(week);
    auto costFn = Interpolator::linearInterpolation(rhsValues[week], costs);
    auto penalty = reservoirManagement.get_penalty(week, endWeek + 1)(level);

    for (double value_fut: X)
    {
        double u = -value_fut + level + reservoir.inflow[week][scenario];
        if ((-reservoir.max_pumping[week] * reservoir.efficiency <= u)
            && (u <= reservoir.max_generating[week] + reservoir.inflow[week][scenario]))
        {
            u = std::min(u, reservoir.max_generating[week]);
            double G = costFn(u);
            if (G + V_fut(value_fut) + penalty < Vu)
            {
                Vu = G + V_fut(value_fut) + penalty;
                xf = value_fut;
            }
        }
    }

    for (double u: rhsValues[week])
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
    for (unsigned int week = startWeek; week <= endWeek; ++week)
    {
        logger->display_message(
          (std::stringstream() << "Computing optimal trajectory, week " << week).str(),
          LogUtils::LOGLEVEL::DEBUG,
          logger->CONTEXT);
        for (unsigned int scenario: scenarios)
        {
            // future costs now coming from previously computed Bellman values
            auto V_fut = [this, &week, &scenario]()
            {
                auto& V_vec = this->bellmanValues[{scenario, week + 1}];
                return [this, &V_vec, &week, &scenario](double x)
                { return Interpolator::linearInterpolation(this->levels, V_vec)(x); };
            };

            // the considered level is the trajectory of the previous week
            // week 0 is always the initial level
            double level = week == 0 ? reservoirManagement.reservoir.initial_level
                                     : trajectory[week][scenario];
            std::tie(
              std::ignore,
              trajectory[week + 1][scenario],
              std::ignore) = solveWeeklyProblemWithCost(week,
                                                        endWeek,
                                                        scenario,
                                                        level, // here: trajectory of the previous
                                                               // week (first week: initial level)
                                                        levels,
                                                        costs[{scenario, week}],
                                                        V_fut());
        }
    }
    logger->display_message("Computed optimal trajectory, for all weeks.",
                            LogUtils::LOGLEVEL::DEBUG,
                            logger->CONTEXT);
    return trajectory;
}
