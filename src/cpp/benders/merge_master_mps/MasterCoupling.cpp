#include <antares-xpansion/benders/merge_master_mps/MasterCoupling.h>
#include <antares-xpansion/xpansion_interfaces/LoggerUtils.h>

#include <json/json.h>

#include <utility>


InvestmentCandidateData MasterCouplingMapGenerator::InvestmentCandidateDataParser(
    const Json::Value& json_node,
    ILoggerXpansion* logger
){
    InvestmentCandidateData candidate_data;
    candidate_data.previous_value = json_node["previous_value"].asString();
    candidate_data.investment_cost = json_node["investment_cost"].asDouble();
    candidate_data.operational_cost = json_node["operational_cost"].asDouble();

    return candidate_data;
};

TrajectoryNodeData MasterCouplingMapGenerator::TrajectoryNodeDataParser(
    const Json::Value& json_node,
    ILoggerXpansion* logger
){
    TrajectoryNodeData node_data;
    node_data.year = json_node["year"].asInt();
    node_data.master_mps_file = json_node["master_mps_file"].asString();
    node_data.structure_file = json_node["structure_file"].asString();
    const auto& subproblem_files = json_node["subproblem_files"];
    for (const auto& subproblem_file : subproblem_files)
    {
        node_data.subproblem_files.push_back(subproblem_file.asString());
    }
    node_data.parent = json_node["parent"].asString();
    node_data.probability = json_node["probability"].asDouble();
    node_data.duration = json_node["duration"].asInt();
    node_data.weighr_factor = json_node["weight_factor"].asDouble();

    const auto& investment_candidates = json_node["investment_candidates"];
    for (const auto& candidate : investment_candidates)
    {
        InvestmentCandidateData candidate_data = InvestmentCandidateDataParser(candidate, logger);
        node_data.investment_candidates[candidate_data.previous_value] = candidate_data;
    }

    return node_data;
};


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

GeneralTrajectoryData MasterCouplingMapGenerator::GeneralTrajectoryDataParser(
    const Json::Value& input,
    ILoggerXpansion* logger
){
    GeneralTrajectoryData general_trajectory_data;
    general_trajectory_data.root_year = input["root_year"].asInt();
    return general_trajectory_data;
}

std::pair<GeneralTrajectoryData, MasterCouplingMap> MasterCouplingMapGenerator::BuildInput(
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

    const auto& general_trajectory_data = GeneralTrajectoryDataParser(input["general_trajectory_data"], logger);

    return std::make_pair(
        general_trajectory_data,
        coupling_map
    );
}
