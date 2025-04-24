#pragma once

#include "antares-xpansion/benders/benders_core/BendersMathLogger.h"
#include "antares-xpansion/benders/benders_core/SubproblemCut.h"
#include "antares-xpansion/benders/benders_core/SubproblemWorker.h"
#include "antares-xpansion/benders/output/JsonWriter.h"
#include "antares-xpansion/xpansion_interfaces/ILogger.h"

using ConstraintMap = std::map<std::string /*constraint name*/, std::vector<double> /*rhs values*/>;
using AreaConstraintMaps = std::map<std::string /*area name*/, ConstraintMap>;
using ConstraintCombos = std::vector<
  std::map<std::string /*constraint name*/, double /*rhs value*/>>;

struct ScenarioAndWeek
{
    int scenario;
    int week;

    bool operator<(const ScenarioAndWeek& other) const
    {
        return std::tie(scenario, week) < std::tie(other.scenario, other.week);
    }
};

/// @brief Class to compute  Stock levels variation
class ValeursUsage
{
public:
    ValeursUsage(Logger logger,
                 std::shared_ptr<Output::JsonWriter> writer,
                 std::filesystem::path path_to_data);
    void launch();

    Output::VariationDeNiveauxDeStockData
      variationDeNiveauxDeStockData; //!< Data to write in the output file

protected:
    void InitSubProblems();
    void Run();
    void ProcessSubproblem(const std::string& subPbName);
    std::map<int, AreaConstraintMaps> GenerateRHSGridValues(std::string subPbName,
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
    void WriteOutput();

protected:
    std::filesystem::path xpansionFolderPath; ///< Path to the xpansion folder
    std::vector<std::string> subPbNames;      ///< List of subproblems names

    SolverLogManager solver_log_manager_;
    Logger _logger;
    std::shared_ptr<Output::JsonWriter> _writer;
};
