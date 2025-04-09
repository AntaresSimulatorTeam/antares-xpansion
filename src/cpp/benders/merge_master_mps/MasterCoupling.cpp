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
    CandidateConstraintData candidate_data;
    candidate_data.max_investment = json_node["max_investment"].asDouble();
    candidate_data.min_investment = json_node["min_investment"].asDouble();

    return candidate_data;
};


TrajectoryNodeData MasterCouplingMapGenerator::TrajectoryNodeDataParser(
    const Json::Value& json_node,
    ILoggerXpansion* logger
){
    TrajectoryNodeData node_data;
    node_data.investment_date = json_node["investment_data"].asInt();
    node_data.lp_folder = json_node["lp_folder"].asString();
    node_data.master_mps_file = json_node["master_mps_file"].asString();
    node_data.structure_file = json_node["structure_file"].asString();
    node_data.parent = json_node["parent"].asString();
    node_data.weight_factor = json_node["weight_factor"].asDouble();

    const auto& candidates_constraints = json_node["constraints"];
    for (const auto& candidate_name : candidates_constraints.getMemberNames())
    {
        const auto& candidate_data = candidates_constraints[candidate_name];
        CandidateConstraintData candidate_constraint_data = CandidateConstraintDataParser(candidate_data, logger);
        node_data.candidate_constraints[candidate_name] = candidate_constraint_data;
    }

    return node_data;
};

MasterCouplingMap MasterCouplingMapGenerator::BuildInput(
    const std::filesystem::path& structure_path,
    ILoggerXpansion* logger  
){
    MasterCouplingMap coupling_map;
    const auto input = parse_json_file(structure_path, logger);

    for (const auto& node_name : input.getMemberNames())
    {
        if (node_name == "general_data")
            continue;
        const auto& node_data = input[node_name];
        TrajectoryNodeData trajectory_node_data = TrajectoryNodeDataParser(node_data, logger);
        coupling_map[node_name] = trajectory_node_data;
    }

    logger->display_message("Master coupling map generated successfully.");
    logger->display_message("Number of nodes: " + std::to_string(coupling_map.size()));

    return coupling_map;
}
