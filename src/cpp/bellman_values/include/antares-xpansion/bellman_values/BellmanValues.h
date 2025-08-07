#pragma once

#include <functional>

#include "antares-xpansion/grid_evaluator/GridEvaluator.h"

/// @brief Class to compute Bellman values
class BellmanValues
{
public:
    BellmanValues(GridEvaluator& gridEvaluator, const ReservoirManagement& reservoirManagement);

    std::vector<std::vector<double>> compute(int startWeek = 1,
                                             int endWeek = 52,
                                             int nbLevels = 10);

    const std::vector<double>& getLevels();

private:
    double solveWeeklyProblemWithReward(int week,
                                        int scenario,
                                        double level,
                                        const std::vector<double>& X,
                                        const std::vector<double>& rewards,
                                        const std::function<double(double)>& V_fut,
                                        std::set<double>& breaking_points);

protected:
    GridEvaluator& gridEvaluator;                   ///< Grid evaluator
    const ReservoirManagement& reservoirManagement; ///< Reservoir management
    std::vector<double> levels;                     ///< Levels of the reservoir
};
