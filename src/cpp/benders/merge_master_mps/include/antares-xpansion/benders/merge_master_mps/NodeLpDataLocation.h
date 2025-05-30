#pragma once

#include <filesystem>
#include <json/value.h>
#include <string>

#include "antares-xpansion/benders/merge_master_mps/MasterStructureKeys.h"

// We separate this struct to a different file to use it in:
// - In MergeMasterTrajectoryMPS
// - In MergeWeightsFileTrajectory
// - In MultipleProblemGeneration when generating the file (this will ensures validity of a
// programmatically generated file)
// -- Both need to read lp folders of nodal studies.
// Refactor probably needed : regroup merge_master_mps and merge_weights_trajectory perhaps ?
//  (& even multiple_problem_generation ?)
// In any case, merge_master_mps should probably not be inside benders/

// Contains the paths and filename to the data resulting from --step problem_generation for a given
// node
struct NodeLpDataLocation
{
    // Constructor for parsing from NodalLpInfo file
    NodeLpDataLocation(const Json::Value& data);

    // Initializes a NodeLpDataLocation object with only lp_folder as non default valued
    NodeLpDataLocation(const std::filesystem::path& path):
        lp_folder(path)
    {
    }

    // Folder containing the generated data
    std::filesystem::path lp_folder;
    // Name of the master problem for this node
    std::string master = MasterStructureKeys::DEFAULT_MASTER_NAME;
    // Structure file of the node
    std::string structure = MasterStructureKeys::DEFAULT_STRUCTURE_FILE;
    // Weights file of the node, might not exist
    std::string weights = MasterStructureKeys::DEFAULT_WEIGHTS_FILE;
    bool has_weights_file{false};
};

typedef std::map<std::string, NodeLpDataLocation> NodesToLpDataLocationMap;

namespace LpDataLocationManager
{
// Load a nodal_lp_info file
NodesToLpDataLocationMap parse_nodal_lp_location_file(const std::filesystem::path& file);

void write_nodal_lp_location_file(const NodesToLpDataLocationMap& lp_info_map,
                                  const std::filesystem::path& file);
} // namespace LpDataLocationManager
