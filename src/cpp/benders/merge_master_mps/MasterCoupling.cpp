#include <antares-xpansion/benders/merge_master_mps/MasterCoupling.h>
#include <antares-xpansion/xpansion_interfaces/LoggerUtils.h>

#include <json/json.h>

#include <utility>


Json::Value parse_json_file(const std::filesystem::path& file_name, ILoggerXpansion* logger) {
    Json::Value _input;
    std::ifstream input_file_l(file_name, std::ifstream::binary);
    Json::CharReaderBuilder builder_l;
    std::string errs;
    if (!parseFromStream(builder_l, input_file_l, &_input, &errs))
    {
        using namespace std::string_literals;
        auto message = LOGLOCATION + "Invalid options file: "s + file_name.string() + "\n" + errs;
        throw InvalidMasterStructureFileException(message);
    }
    return _input;
}

CandidateConstraintData MasterCouplingMapGenerator::CandidateConstraintDataParser(
    const Json::Value& json_node,
    ILoggerXpansion* logger
){
    using namespace MasterCouplingConstants;

    CandidateConstraintData candidate_data;
    candidate_data.max_investment = json_node[KEY_MAX_INVESTMENT].asDouble();
    candidate_data.min_investment = json_node[KEY_MIN_INVESTMENT].asDouble();

    return candidate_data;
};


TrajectoryNodeData MasterCouplingMapGenerator::TrajectoryNodeDataParser(
    const Json::Value& json_node,
    ILoggerXpansion* logger
){
    using namespace MasterCouplingConstants;

    TrajectoryNodeData node_data;
    node_data.investment_date = json_node[KEY_INVESTMENT_DATE].asInt();
    node_data.lp_folder = json_node[KEY_LP_FOLDER].asString();
    node_data.master_mps_file = json_node[KEY_MASTER_MPS_FILE].asString();
    node_data.structure_file = json_node[KEY_STRUCTURE_FILE].asString();
    node_data.parent = json_node[KEY_PARENT].asString();
    node_data.weight_factor = json_node[KEY_WEIGHT_FACTOR].asDouble();
    
    // If a MASTER_NAME is given, set it (used when accesing the structure file)
    if (json_node.isMember(KEY_MASTER_NAME))
    {
        node_data.master_name = json_node[KEY_MASTER_NAME].asString();
    }

    const auto& candidates_constraints = json_node[KEY_CONSTRAINTS];
    for (const auto& candidate_name : candidates_constraints.getMemberNames())
    {
        const auto& candidate_data = candidates_constraints[candidate_name];
        CandidateConstraintData candidate_constraint_data = CandidateConstraintDataParser(candidate_data, logger);
        node_data.candidate_constraints[candidate_name] = candidate_constraint_data;
    }

    return node_data;
};

TrajectoryGlobalData MasterCouplingMapGenerator::TrajectoryGlobalDataParser(
    const Json::Value& json_node,
    ILoggerXpansion* logger
){
    using namespace MasterCouplingConstants;

    TrajectoryGlobalData trajectory_data;
    const auto& initial_capacities = json_node[KEY_INITIAL_CAPACITIES];
    // Set a default default value
    trajectory_data.initial_capacities[KEY_DEFAULT] = 0;
    for (const auto& candidate_name : initial_capacities.getMemberNames())
    {
        trajectory_data.initial_capacities[candidate_name] = initial_capacities[candidate_name].asDouble();
    }
    return trajectory_data;
};

std::pair<TrajectoryGlobalData, MasterCouplingMap> MasterCouplingMapGenerator::BuildInput(
    const std::filesystem::path& structure_path,
    ILoggerXpansion* logger  
){
    using namespace MasterCouplingConstants;
    
    MasterCouplingMap coupling_map;
    const auto input = parse_json_file(structure_path, logger);

    for (const auto& node_name : input.getMemberNames())
    {
        if (node_name == KEY_DATA)
            continue;
        const auto& node_data = input[node_name];
        TrajectoryNodeData trajectory_node_data = TrajectoryNodeDataParser(node_data, logger);
        coupling_map[node_name] = trajectory_node_data;
    }

    logger->display_message("Master coupling map generated successfully.");
    logger->display_message("Number of nodes: " + std::to_string(coupling_map.size()));

    const auto& general_data = input[KEY_DATA];
    TrajectoryGlobalData trajectory_data = TrajectoryGlobalDataParser(general_data, logger);
    
    return std::make_pair(trajectory_data, coupling_map);
}
