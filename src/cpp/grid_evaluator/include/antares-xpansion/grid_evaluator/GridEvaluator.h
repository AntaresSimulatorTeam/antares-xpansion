#pragma once

#include <functional>

#include <antares/solver/lps/LpsFromAntares.h>

#include "antares-xpansion/benders/benders_core/SubproblemWorker.h"
#include "antares-xpansion/benders/output/JsonWriter.h"
#include "antares-xpansion/grid_evaluator/GridCollection.h"
#include "antares-xpansion/lpnamer/model/Problem.h"
#include "antares-xpansion/xpansion_interfaces/ILogger.h"

constexpr char GRID_EVALUATOR_LOGGER_CONTEXT[] = "GridEvaluator";

/// @brief vector of maps (key constraint name, value rhs value)
using ConstraintCombos = std::vector<std::map<std::string, double>>;

struct GridPointResult
{
    double cost;
    std::map<AreaName, double> dual{};
};

/// @brief Class to compute Stock levels variation
class GridEvaluator
{
public:
    GridEvaluator(Logger logger,
                  std::map<Antares::Solver::WeeklyProblemId, std::shared_ptr<Problem>> problems,
                  GridDefinition& grid_definition,
                  std::string solverName,
                  std::filesystem::path studyDir,
                  int nbThreads = 1);
    virtual std::map<Output::PointWeekScenarioKey, GridPointResult> ComputeCostsAndDuals();

private:
    Output::ConcurrentInsertionMap<Output::PointWeekScenarioKey, GridPointResult>
      variationDeNiveauxDeStockResults;

protected:
    void Run();
    void ProcessSubproblem(const Antares::Solver::WeeklyProblemId,
                           std::shared_ptr<Problem> subProblem);
    void SetConstraintsRHSValues(const std::map<std::string, double>& rhsValues,
                                 std::shared_ptr<Problem> subProblem);
    GridPointResult SolveSubproblem(std::shared_ptr<Problem> subProblem, Point subPbCombo);
    std::string GetConstraintName(const Antares::Solver::WeeklyProblemId id,
                                  const std::string& area,
                                  const std::string& constraint) const;
    ConstraintCombos GenerateConstraintProduct(const ConstraintMap& constraints);
    ConstraintCombos GenerateSubPbCombos(const Antares::Solver::WeeklyProblemId subProblemId,
                                         const AreaConstraintMaps& areas);

protected:
    std::map<Antares::Solver::WeeklyProblemId, std::shared_ptr<Problem>>
      problems;                     ///< map of subproblems
    GridDefinition& gridDefinition; ///< Grid definition
    std::string solverName;         ///< Solver name

    int nbThreads; ///< Number of threads to use

    SolverLogManager solver_log_manager;
    std::filesystem::path studyDir; // the path to the study dir, in order to write files there
    Logger logger;

    friend class BellmanValues;
};
