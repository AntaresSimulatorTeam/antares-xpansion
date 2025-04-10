#pragma once

#include <string>
#include <vector>
#include <map>

#include <antares-xpansion/benders/merge_master_mps/MasterCoupling.hxx>

#include <antares-xpansion/xpansion_interfaces/ILogger.h>
#include <antares-xpansion/benders/benders_core/common.h>

#include <json/json.h>

// // Define the keys expected in the master structure file that links the nodes in the trajectory
// #define KEY_DATA "data"
//     #define KEY_INITIAL_CAPACITIES "initial_capacities"
//         #define KEY_DEFAULT "default"

//     #define KEY_INVESTMENT_DATE "investment_date"
//     #define KEY_LP_FOLDER "lp_folder"
//     #define KEY_MASTER_MPS_FILE "master_mps_file"
//     #define KEY_STRUCTURE_FILE "structure_file"
//     #define KEY_PARENT "parent"
//     #define KEY_WEIGHT_FACTOR "weight_factor"
//     #define KEY_CONSTRAINTS "constraints"
//         #define KEY_MAX_INVESTMENT "max_investment"
//         #define KEY_MIN_INVESTMENT "max_decommissioning"

// // Forbidden name for a node :
// #define ROOT_NAME "ROOT"


class InvalidMasterStructureFileException: public std::runtime_error
{
public:
    explicit InvalidMasterStructureFileException(const std::string& arg):
        std::runtime_error(arg) {}
};

struct CandidateConstraintData
{
    double max_investment;
    double min_investment;
};

struct TrajectoryGlobalData
{
    std::map<std::string, double> initial_capacities;
};

struct TrajectoryNodeData {
    int investment_date;
    std::string lp_folder;
    std::string master_mps_file;
    std::string structure_file;
    std::string parent = master_structure::ROOT_NAME;
    std::string master_name = master_structure::DEFAULT_MASTER_NAME;
    double weight_factor = 1.0;
    std::map<std::string, CandidateConstraintData> candidate_constraints;
};


typedef std::map<std::string, TrajectoryNodeData> MasterCouplingMap;


class MasterCouplingMapGenerator
{
public:

    static CandidateConstraintData CandidateConstraintDataParser(
        const Json::Value& json_node,
        ILoggerXpansion* logger
    );

    static TrajectoryNodeData TrajectoryNodeDataParser(
        const Json::Value& json_node,
        ILoggerXpansion* logger
    );

    static TrajectoryGlobalData TrajectoryGlobalDataParser(
        const Json::Value& json_node,
        ILoggerXpansion* logger
    );

    static std::pair<TrajectoryGlobalData, MasterCouplingMap> BuildInput(
        const std::filesystem::path& structure_path,
        ILoggerXpansion* logger
    );

};