#pragma once

#include "antares-xpansion/benders/logger/User.h"
#include "antares-xpansion/benders/merge_master_mps/NodeLpDataLocation.h"

constexpr char MERGE_WEIGHTS_CONTEXT[] = "Trajectory Merged Weights Generation";
constexpr char WEIGHT_SUM_KEY[] = "WEIGHT_SUM";

class MergeWeightsTrajectory
{
private:
    typedef std::map<std::string, double> WeightsMap;

public:
    MergeWeightsTrajectory(const std::filesystem::path& master_merger_info,
                           const std::filesystem::path& nodal_file,
                           const std::filesystem::path& output_file,
                           Logger logger):
        master_merger_info_file_(master_merger_info),
        nodal_lp_folder_file_(nodal_file),
        output_filepath_(output_file),
        logger_(std::move(logger))
    {
    }

    void load_input_files();
    void generate_merged_weights_file();
    void write_merged_weights_file() const;

private:
    std::filesystem::path master_merger_info_file_;
    std::filesystem::path nodal_lp_folder_file_;
    std::filesystem::path output_filepath_;

    Logger logger_;
    NodesToLpDataLocationMap nodes_lp_info_;
    WeightsMap nodes_weights_;

    WeightsMap merged_subproblem_weights_;
};
