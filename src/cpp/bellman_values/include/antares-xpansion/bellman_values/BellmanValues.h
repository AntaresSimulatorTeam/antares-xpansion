#pragma once

#include <functional>

#include "antares-xpansion/evaluator/GridEvaluator.h"

/// @brief Class to compute Bellman values
class BellmanValues
{
public:
    BellmanValues(GridEvaluator& gridEvaluator,
                  const ReservoirManagement& reservoirManagement,
                  Logger logger);

    std::vector<std::vector<double>> compute(int nbLevels = 10);

    const std::vector<double>& getLevels() const;

    std::vector<std::vector<double>> computeOptimalTrajectories();

private:
    std::tuple<double, double, double> solveWeeklyProblemWithCost(
      int week,
      int endWeek,
      int scenario,
      double level,
      const std::vector<double>& X,
      const std::vector<double>& costs,
      const std::function<double(double)>& V_fut);

    // for multistock cases, we need to store the costs and bellman values
    std::map<Antares::Solver::WeeklyProblemId, std::vector<double>> costs;
    std::map<Antares::Solver::WeeklyProblemId, std::vector<double>> bellmanValues;

protected:
    Logger logger;

    GridEvaluator& gridEvaluator;                   ///< Grid evaluator
    const ReservoirManagement& reservoirManagement; ///< Reservoir management
    std::vector<double> levels;                     ///< Levels of the reservoir
};
