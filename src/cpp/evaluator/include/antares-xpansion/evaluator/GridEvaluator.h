#pragma once

#include <functional>

#include <antares/solver/lps/LpsFromAntares.h>

#include "antares-xpansion/benders/benders_core/SubproblemWorker.h"
#include "antares-xpansion/benders/output/JsonWriter.h"
#include "antares-xpansion/evaluator/Evaluator.h"
#include "antares-xpansion/evaluator/GridCollection.h"
#include "antares-xpansion/lpnamer/model/Problem.h"
#include "antares-xpansion/xpansion_interfaces/ILogger.h"

constexpr char GRID_EVALUATOR_LOGGER_CONTEXT[] = "GridEvaluator";

/// @brief vector of maps (key constraint name, value rhs value)
using ConstraintCombos = std::vector<std::map<std::string, double>>;
using namespace PlainData;

/// @brief Class to compute Stock levels variation
class GridEvaluator: public Evaluator
{
public:
    GridEvaluator(Logger logger,
                  std::map<Antares::Solver::WeeklyProblemId, std::shared_ptr<Problem>> problems,
                  GridDefinition& grid_definition,
                  std::string solverName,
                  std::filesystem::path studyDir,
                  int nbThreads = 1);
    // virtual function to be overridable for the tests
    virtual std::map<Output::PointWeekScenarioKey, SubProblemData> ComputeCostsAndDuals();

private:
    Output::ConcurrentInsertionMap<Output::PointWeekScenarioKey, SubProblemData>
      variationDeNiveauxDeStockResults;

protected:
    void ProcessSubproblem(const Antares::Solver::WeeklyProblemId,
                           std::shared_ptr<Problem> subProblem) override;

    ConstraintCombos GenerateConstraintProduct(const ConstraintMap& constraints);
    ConstraintCombos GenerateSubPbCombos(const Antares::Solver::WeeklyProblemId subProblemId,
                                         const AreaConstraintMaps& areas);

protected:
    GridDefinition& gridDefinition; ///< Grid definition

    friend class BellmanValues;
};
