#pragma once

#include "antares-xpansion/benders/benders_core/BendersMathLogger.h"
#include "antares-xpansion/benders/benders_core/SubproblemCut.h"
#include "antares-xpansion/benders/benders_core/SubproblemWorker.h"
#include "antares-xpansion/xpansion_interfaces/ILogger.h"

/*!
 * \class GridSearch
 * \brief Class use run the GridSearch algorithm
 */
class GridSearch
{
public:
    GridSearch(Logger logger, std::shared_ptr<Output::OutputWriter> writer);
    GridSearch(Logger logger, std::shared_ptr<Output::OutputWriter> writer, std::filesystem::path path_to_data);

    void launch();

    struct GridPointData
    {
        double investment_cost;
        double operational_cost;
        double overall_cost;
        Point point;
        std::vector<std::vector<double>> solution;
    };

    std::vector<GridPointData> gridPointData;

protected:
    void Run();
    void InitializeProblems();
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
    std::filesystem::path xpansionFolderPath = std::filesystem::path(
      "/home/workspace/antares-xpansion-generic/data_test/mini_instance_LP");
    std::vector<Point> gridPoints;
    VariableMap _problem_to_id;
    CouplingMap coupling_map;
    SubproblemsMapPtr subproblem_map;
    StrVector subproblems;
    Point current_point_;
    std::map<std::string, double> investCostPerMwPerYear;

    SolverLogManager solver_log_manager_;
    Logger _logger;
    std::shared_ptr<Output::OutputWriter> _writer;
};
