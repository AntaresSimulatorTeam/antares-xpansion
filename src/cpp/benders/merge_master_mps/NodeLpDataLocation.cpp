#include "antares-xpansion/benders/merge_master_mps/NodeLpDataLocation.h"

#include "antares-xpansion/benders/benders_core/common.h"

NodeLpDataLocation::NodeLpDataLocation(
    const Json::Value& data
)
{
    using namespace MasterStructureKeys;
    lp_folder = data[KEY_LP_FOLDER].asString();
    if (data.isMember(KEY_MASTER_MPS_FILE))
    {   
        master = data[KEY_MASTER_MPS_FILE].asString();
    }
    if (data.isMember(KEY_STRUCTURE_FILE))
    {    
        structure = data[KEY_STRUCTURE_FILE].asString();
    }
    if (data.isMember(KEY_WEIGHTS_FILE))
    {
        weights = data[KEY_WEIGHTS_FILE].asString();
    }
}

NodesToLpDataLocationMap LpDataLocationLoader::parse_nodal_lp_location_file(const std::filesystem::path& file)
{
    NodesToLpDataLocationMap output;
    const auto raw_input = get_json_file_content(file);
    for (const auto& node_name: raw_input.getMemberNames())
    {
        const auto& node_lp_path = raw_input[node_name];
        output.emplace(
            std::make_pair(node_name, NodeLpDataLocation(node_lp_path))
        );
    }

    return output;
}