#pragma once

#include "antares-xpansion/benders/merge_master_mps/NodeLpDataLocation.h"
#include "antares-xpansion/benders/logger/User.h"

constexpr char MERGE_WEIGHTS_CONTEXT[] = "Trajectory Merged Weights Generation";

class MergeWeightsTrajectory
{

public:
    MergeWeightsTrajectory(const std::filesystem::path& master_structure, 
                           const std::filesystem::path& nodal_file,
                           const std::filesystem::path& output_file,
                           Logger logger):
        master_structure_file_(master_structure),
        nodal_lp_folder_file_(nodal_file),
        output_filepath_(output_file),
        logger_(std::move(logger))
    {
    };

    void generate_merged_weights_file();

private:
    std::filesystem::path master_structure_file_;
    std::filesystem::path nodal_lp_folder_file_;
    std::filesystem::path output_filepath_;

    Logger logger_;
    NodesToLpDataLocationMap nodes_lp_paths_;
};