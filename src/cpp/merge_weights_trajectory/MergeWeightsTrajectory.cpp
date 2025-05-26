#include "antares-xpansion/merge_weights_trajectory/MergeWeightsTrajectory.h"

#include <fstream>
#include <json/reader.h>

#include "antares-xpansion/benders/benders_core/CouplingMapGenerator.h"
#include "antares-xpansion/benders/benders_core/common.h"
#include "antares-xpansion/benders/merge_master_mps/MasterStructureKeys.h"
#include "antares-xpansion/lpnamer/input_reader/GeneralDataReader.h"
#include "antares-xpansion/xpansion_interfaces/StringManip.h"

namespace
{
void check_format(const std::vector<std::string>& split)
{
    const std::string file_format_error{
      "Weights file should have two columns separated by ' ' : \n "
      "subproblem_file weight \n"
      "subproblem_file2 weight"};
    if (split.size() != 2)
    {
        std::cerr << file_format_error << std::endl;
        std::exit(1);
    }
}

/*
    File should be in two columns :
    subproblem_file weight
    ...
    WEIGHT_SUM sum
    Where WEIGHT_SUM is the sum of weights over all MC years (but not over the weeks !)
    Note that WEIGHT_SUM is thus a disallowed name for a subproblem file (should be ok !)
    Returns a map : subproblem_file -> subproblem_weight
                    WEIGHT_SUM -> sum
*/
std::map<std::string, double> load_weights_map(const std::filesystem::path& path)
{
    std::ifstream f(path);
    std::string line;
    std::map<std::string, double> output;

    // Nodal studies input paths
    while (std::getline(f, line))
    {
        auto split = StringManip::split(line, " ");
        check_format(split);
        output[split[0]] = std::stod(split[1]);
    }

    return output;
}
} // namespace

void MergeWeightsTrajectory::load_input_files()
{
    using namespace MasterStructureKeys;
    // Read the master structure file and extract only the relevant information
    const auto master_structure_data = get_json_file_content(master_structure_file_);
    const auto& tree_data = master_structure_data[KEY_TREE];

    for (const auto& node_name: tree_data.getMemberNames())
    {
        double node_weight = tree_data[node_name][KEY_NODE_WEIGHT].asDouble();
        nodes_weights_[node_name] = node_weight;
    }

    // Lp paths & relevant files data
    nodes_lp_info_ = LpDataLocationManager::parse_nodal_lp_location_file(nodal_lp_folder_file_);
}

void MergeWeightsTrajectory::generate_merged_weights_file()
{
    for (const auto& [node, lp_info]: nodes_lp_info_)
    {
        // If there are custom weights for this node
        auto potential_weights_file = lp_info.lp_folder / lp_info.weights;
        if (std::filesystem::exists(potential_weights_file))
        {
            logger_->display_message("Node '" + node + "' has a custom weight file, parsing file : "
                                       + potential_weights_file.string(),
                                     LogUtils::LOGLEVEL::INFO,
                                     MERGE_WEIGHTS_CONTEXT);

            const auto nodal_weights = load_weights_map(potential_weights_file);
            for (const auto& [subproblem, weight]: nodal_weights)
            {
                if (subproblem == WEIGHT_SUM_KEY)
                {
                    continue;
                }
                auto full_path = lp_info.lp_folder / subproblem;
                auto merged_weight = (weight / nodal_weights.at(WEIGHT_SUM_KEY))
                                     * nodes_weights_.at(node);
                merged_subproblem_weights_[full_path.string()] = merged_weight;
            }
        }
        // Otherwise, use uniform weights for the subproblems
        else
        {
            auto structure_file = lp_info.lp_folder / lp_info.structure;
            logger_->display_message("Node '" + node
                                       + "' has no weights file, parsing structure file : "
                                       + structure_file.string(),
                                     LogUtils::LOGLEVEL::INFO,
                                     MERGE_WEIGHTS_CONTEXT);

            CouplingMap node_structure = CouplingMapGenerator::BuildInput(structure_file,
                                                                          logger_.get(),
                                                                          MERGE_WEIGHTS_CONTEXT);

            // Get the number of MC_YEARS
            const auto settings_dir = lp_info.lp_folder / ".." / ".." / ".." / "settings";
            const auto general_data_file = settings_dir / "generaldata.ini";
            auto ini_reader_logger = std::make_shared<
              ProblemGenerationLog::ProblemGenerationLogger>(LogUtils::LOGLEVEL::INFO);
            auto genera_data_reader = GeneralDataIniReader(general_data_file, ini_reader_logger);
            std::vector<int> active_years = genera_data_reader.GetActiveYears();
            int nb_years = active_years.size();

            logger_->display_message("After reading Antares settings, node '" + node
                                       + "' was found to have " + std::to_string(nb_years)
                                       + " MC years.",
                                     LogUtils::LOGLEVEL::INFO,
                                     MERGE_WEIGHTS_CONTEXT);

            for (const auto& [subproblem, _]: node_structure)
            {
                if (subproblem == lp_info.master)
                {
                    continue;
                }
                auto full_path = lp_info.lp_folder / subproblem;
                auto merged_weight = (1 / static_cast<double>(nb_years)) * nodes_weights_[node];
                merged_subproblem_weights_[full_path.string()] = merged_weight;
            }
        }
    }
}

void MergeWeightsTrajectory::write_merged_weights_file() const
{
    std::ofstream weight_file;
    weight_file.open(output_filepath_);

    logger_->display_message("Writing merged weights to file : " + output_filepath_.string(),
                             LogUtils::LOGLEVEL::INFO,
                             MERGE_WEIGHTS_CONTEXT);

    for (const auto& [subproblem, weight]: merged_subproblem_weights_)
    {
        weight_file << subproblem << " " << weight << std::endl;
    }

    // WEIGHT_SUM
    weight_file << WEIGHT_SUM_KEY << " " << 1.0 << std::endl;
    weight_file.close();
}
