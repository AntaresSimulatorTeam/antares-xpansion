#include "antares-xpansion/benders/merge_master_mps/NodeLpDataLocation.h"

#include <ctime>
#include <iomanip>
#include <json/writer.h>
#include <sstream>

#include "antares-xpansion/benders/benders_core/common.h"

NodeLpDataLocation::NodeLpDataLocation(const Json::Value& data)
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

NodeLpDataLocation::NodeLpDataLocation(std::filesystem::path path):
    lp_folder(std::move(path))
{
}

NodesToLpDataLocationMap LpDataLocationManager::parse_nodal_lp_location_file(
  const std::filesystem::path& file)
{
    NodesToLpDataLocationMap output;
    for (const auto raw_input = get_json_file_content(file);
         const auto& node_name: raw_input.getMemberNames())
    {
        if (node_name != MasterStructureKeys::KEY_METADATA)
        {
            const auto& node_lp_path = raw_input[node_name];
            output.emplace(std::make_pair(node_name, NodeLpDataLocation(node_lp_path)));
        }
    }

    return output;
}

void LpDataLocationManager::write_nodal_lp_location_file(
  const NodesToLpDataLocationMap& lp_info_map,
  const std::filesystem::path& filepath)
{
    using namespace MasterStructureKeys;
    Json::Value output;

    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");
    auto str = oss.str();

    output[KEY_METADATA]["written"] = str;

    for (const auto& [name, lp_info]: lp_info_map)
    {
        output[name][KEY_LP_FOLDER] = lp_info.lp_folder.string();
        // Optionnal keys are:
        output[name][KEY_MASTER_MPS_FILE] = lp_info.master;
        output[name][KEY_STRUCTURE_FILE] = lp_info.structure;
        output[name][KEY_WEIGHTS_FILE] = lp_info.weights;
    }

    Json::StreamWriterBuilder builder;
    builder["commentStyle"] = "None";
    builder["indentation"] = "   ";
    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    std::ofstream outputFileStream(filepath);
    writer->write(output, &outputFileStream);
    std::cout << "Successfully written lp_paths to file : " << filepath.string() << std::endl;
}
