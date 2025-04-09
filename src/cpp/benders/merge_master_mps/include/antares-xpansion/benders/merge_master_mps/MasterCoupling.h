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


struct TrajectoryNodeData {
    int investment_date;
    std::string lp_folder;
    std::string master_mps_file;
    std::string structure_file;
    std::string parent = "root";
    double weight_factor = 1.0;
};


typedef std::map<std::string, TrajectoryNodeData> MasterCouplingMap;


class MasterCouplingMapGenerator
{
public:
    static TrajectoryNodeData TrajectoryNodeDataParser(
        const Json::Value& json_node,
        ILoggerXpansion* logger
    );
    static MasterCouplingMap BuildInput(
        const std::filesystem::path& structure_path,
        ILoggerXpansion* logger
    );

};