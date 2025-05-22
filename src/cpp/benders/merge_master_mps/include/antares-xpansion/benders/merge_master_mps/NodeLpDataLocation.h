#pragma once

#include <json/value.h>
#include <string>
#include <filesystem>

#include "antares-xpansion/benders/merge_master_mps/MasterStructureKeys.h"

// We separate this struct to a different file to use it both:
// - In MergeMasterTrajectoryMPS
// - In MergeWeightsFileTrajectory
// -- Both need to read lp folders of nodal studies.

// Refactor probably needed : regroup merge_master_mps and merge_weights trajectory perhaps ?
// In any case, merge_master_mps should probably not be inside benders/

// Contains the paths and filename to the data resulting from --step problem_generation for a given node
struct NodeLpDataLocation
{  
    NodeLpDataLocation(const Json::Value& data);
    std::filesystem::path lp_folder;
    std::string master = MasterStructureKeys::DEFAULT_MASTER_NAME;
    std::string structure = MasterStructureKeys::DEFAULT_STRUCTURE_FILE;
    std::string weights = MasterStructureKeys::DEFAULT_WEIGHTS_FILE;
};

typedef std::map<std::string, NodeLpDataLocation> NodesToLpDataLocationMap;

namespace LpDataLocationLoader
{
    NodesToLpDataLocationMap parse_nodal_lp_location_file(const std::filesystem::path& file);
} // namespace LpDataLocationLoader