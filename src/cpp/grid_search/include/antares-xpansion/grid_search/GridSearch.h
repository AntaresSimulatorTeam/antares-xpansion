#pragma once

#include "antares-xpansion/benders/benders_core/BendersMathLogger.h"
#include "antares-xpansion/benders/benders_core/SubproblemCut.h"
#include "antares-xpansion/benders/benders_core/SubproblemWorker.h"
#include "antares-xpansion/benders/output/JsonWriter.h"
#include "antares-xpansion/xpansion_interfaces/ILogger.h"

/*!
 * \class GridSearch
 * \brief Class use run the GridSearch algorithm
 */
class GridSearch
{
public:
    GridSearch(Logger logger,
               std::shared_ptr<Output::JsonWriter> writer,
               std::filesystem::path path_to_data);

    void launch();

    Output::GridPointsData gridPointsData;

protected:
    void Run();
    void InitializeProblems();
    void ComputeWeights();
    void SetGridPoints();
    void InitCouplingMap();
    void MatchProblemToId();
    std::filesystem::path GetSubproblemPath(const std::string& slave_name) const;
    std::filesystem::path GetMasterProblemPath() const;
    void AddSubproblem(const std::pair<std::string, VariableMap>& kvp);
    std::vector<double> SolveSubproblem(PlainData::SubProblemData& subproblem_data,
                                        const std::string& name,
                                        const std::shared_ptr<SubproblemWorker>& worker);
    void SetInvestmentCostPerMwPerYear(const std::filesystem::path& path_to_mps);

protected:
    std::filesystem::path xpansionFolderPath;
    std::vector<Point> gridPoints;
    VariableMap _problem_to_id;
    CouplingMap coupling_map;
    SubproblemsMapPtr subproblem_map;
    std::vector<std::string> subproblems;
    Point current_point_;
    std::map<std::string, double> investCostPerMwPerYear;
    int nbMonteCarloYears = 1;
    std::map<std::string, double> weights;

    SolverLogManager solver_log_manager_;
    Logger _logger;
    std::shared_ptr<Output::JsonWriter> _writer;
};
