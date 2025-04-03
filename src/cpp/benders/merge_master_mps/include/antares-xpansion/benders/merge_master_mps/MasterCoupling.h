#pragma once

#include <string>
#include <vector>
#include <map>

#include <antares-xpansion/xpansion_interfaces/ILogger.h>
#include <antares-xpansion/benders/benders_core/common.h>

#include <json/json.h>


class InvalidMasterStructureFileException: public std::runtime_error
{
public:
    explicit InvalidMasterStructureFileException(const std::string& arg):
        std::runtime_error(arg) {}
};

struct GeneralTrajectoryData {
    int root_year;
};

struct InvestmentCandidateData {
    // Link to the previous variable, 
    // should contain the name of the variable in the parent
    std::string previous_value;
    double investment_cost;
    double operational_cost;
};

typedef std::map<std::string, InvestmentCandidateData> InvestmentCandidatesMap;

struct TrajectoryNodeData {
    int year;
    std::string master_mps_file;
    std::string structure_file;
    std::vector<std::string> subproblem_files;
    std::string parent = "root";
    double probability = 1.0;
    int duration = 0;
    double weighr_factor = 1.0;
    InvestmentCandidatesMap investment_candidates;
};


typedef std::map<std::string, TrajectoryNodeData> MasterCouplingMap;


class MasterCouplingMapGenerator
{
public:
    static InvestmentCandidateData InvestmentCandidateDataParser(
        const Json::Value& json_node,
        ILoggerXpansion* logger
    );
    static TrajectoryNodeData TrajectoryNodeDataParser(
        const Json::Value& json_node,
        ILoggerXpansion* logger
    );
    static GeneralTrajectoryData GeneralTrajectoryDataParser(
        const Json::Value& json_node,
        ILoggerXpansion* logger
    );
    static std::pair<GeneralTrajectoryData, MasterCouplingMap> BuildInput(
        const std::filesystem::path& structure_path,
        ILoggerXpansion* logger
    );

};