#pragma once

#include "antares-xpansion/benders/benders_core/BendersMathLogger.h"
#include "antares-xpansion/benders/benders_core/SubproblemCut.h"
#include "antares-xpansion/benders/benders_core/SubproblemWorker.h"
#include "antares-xpansion/benders/output/JsonWriter.h"
#include "antares-xpansion/xpansion_interfaces/ILogger.h"

// Define type for the map of constraints
using ConstraintMap = std::map<std::string /*constraint name*/, std::vector<double> /*rhs values*/>;
using AreaConstraintMaps = std::map<std::string /*area name*/, ConstraintMap>;
using SubPbConstraintMaps = std::map<std::string /*subPb name*/, AreaConstraintMaps>;

/*!
 * \class ValeursUsage
 * \brief Class use run the Valeurs d'usage use case
 */
class ValeursUsage
{
public:
    ValeursUsage(Logger logger,
                 std::shared_ptr<Output::JsonWriter> writer,
                 std::filesystem::path path_to_data);

    void launch();

    Output::GridPointsData gridPointsData;

protected:
    void Run();
    void InitSubProblems();
    void SetConstraintsRHSValues();
    void GenerateRHSGridValues();
    void SetConstraintsRHSValuesForPb(const std::string& pbName,
                                      const std::map<std::string, double>& rhsValues);
    std::filesystem::path GetSubproblemPath(const std::string& slave_name) const;
    void AddSubproblem(const std::string& problem_name);
    std::vector<double> SolveSubproblem(PlainData::SubProblemData& subproblem_data,
        const std::string& subPbName);
    std::string getConstraintName(const std::string& subPbName,
                                  const std::string& area,
                                  const std::string& constraint) const;
    int getWeekFromPbName(const std::string& pbName) const;
    void generateConstraintProduct(
      const std::map<std::string, std::vector<double>>& constraints,
      std::map<std::string, double>& current,
      std::map<std::string, std::vector<double>>::const_iterator it,
      const std::function<void(const std::map<std::string, double>&)>& func);
    void generateAreaProduct(const std::string subPbName,
                             const AreaConstraintMaps& areas,
                             std::map<std::string, double>& current,
                             AreaConstraintMaps::const_iterator it,
                             const std::function<void(const std::map<std::string, double>&)>& func);

protected:
    std::filesystem::path xpansionFolderPath;
    std::map<std::string /*subproblem name*/, AreaConstraintMaps> subPbAreaConstraintsMaps;
    SubproblemsMapPtr subproblem_map;
    StrVector subproblems;

    SolverLogManager solver_log_manager_;
    Logger _logger;
    std::shared_ptr<Output::JsonWriter> _writer;
};
