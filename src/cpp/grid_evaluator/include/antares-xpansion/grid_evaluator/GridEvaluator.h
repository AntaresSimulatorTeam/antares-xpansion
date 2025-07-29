#pragma once

#include <functional>

#include "antares-xpansion/benders/benders_core/SubproblemWorker.h"
#include "antares-xpansion/benders/output/JsonWriter.h"
#include "antares-xpansion/grid_evaluator/GridCollection.h"
#include "antares-xpansion/xpansion_interfaces/ILogger.h"

/// @brief key constraint name, value vector of rhs values
using ConstraintMap = std::map<std::string, std::set<double>>;
/// @brief key area name, value constraint map
using AreaConstraintMaps = std::map<std::string, ConstraintMap>;
/// @brief vector of maps (key constraint name, value rhs value)
using ConstraintCombos = std::vector<std::map<std::string, double>>;

/// @brief Class to compute Stock levels variation
class GridEvaluator
{
public:
    GridEvaluator(Logger logger,
                  std::shared_ptr<Output::JsonWriter> writer,
                  std::filesystem::path path_to_mps,
                  GridDefinition& grid_definition,
                  ProblemsFormat data_format,
                  int nbThreads = 1);
    std::map<Output::PointWeekScenarioKey, double> ComputeRewards();

private:
    Output::ConcurrentInsertionMap<Output::PointWeekScenarioKey, double>
      variationDeNiveauxDeStockData;

protected:
    std::vector<std::string> InitSubProblems(const GridDefinition& grid_definition);
    void Run(const std::vector<std::string>& subPbNames, GridDefinition& grid_definition);
    void ProcessSubproblem(const std::string& subPbName, GridDefinition& grid_definition);
    void ProcessGridParallel(const std::vector<std::string>& subPbNames,
                             GridDefinition& grid_definition,
                             int nbThreads);
    AreaConstraintMaps GenerateRHSGridValues(std::string subPbName,
                                             GridDefinition& grid_definition,
                                             SubproblemWorkerPtr subPbWorker);
    void SetConstraintsRHSValues(const std::map<std::string, double>& rhsValues,
                                 SubproblemWorkerPtr subPbWorker);
    std::filesystem::path GetSubproblemPath(const std::string& subPbName) const;
    SubproblemWorkerPtr AddSubproblem(const std::string& subPbName);
    double SolveSubproblem(SubproblemWorkerPtr subPbWorker);
    std::string GetConstraintName(const std::string& subPbName,
                                  const std::string& area,
                                  const std::string& constraint) const;
    ScenarioAndWeek GetPbInfo(const std::string& pbName) const;
    ConstraintCombos GenerateConstraintProduct(const ConstraintMap& constraints);
    ConstraintCombos GenerateSubPbCombos(const std::string& subPbName,
                                         const AreaConstraintMaps& areas);

protected:
    std::filesystem::path mpsPath;  ///< Path to the xpansion folder
    GridDefinition& gridDefinition; ///< Grid definition

    int nbScenarios = 0; ///< Number of scenarios

    ProblemsFormat problemsFormat; ///< Format of the problems
    int nbThreads;                 ///< Number of threads to use

    SolverLogManager solver_log_manager;
    Logger logger;
    std::shared_ptr<Output::JsonWriter> writer;

    friend class BellmanValues;
};
