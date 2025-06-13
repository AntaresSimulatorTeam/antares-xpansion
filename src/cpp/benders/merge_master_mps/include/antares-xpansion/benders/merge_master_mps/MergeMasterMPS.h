#pragma once

#include <optional>

#include "antares-xpansion/benders/merge_master_mps/MasterStructureKeys.h"
#include "antares-xpansion/benders/merge_master_mps/NodeLpDataLocation.h"
#include "antares-xpansion/benders/merge_mps/MergeMPS.h"

constexpr char TRAJECTORY_LOGGER_CONTEXT[] = "MergeMasterTrajectoryMPS";

// Probably needs to be changed
class InvalidMasterStructureFileException: public std::runtime_error
{
public:
    explicit InvalidMasterStructureFileException(const std::string& arg):
        std::runtime_error(arg)
    {
    }
};

class MergeMasterTrajectoryMPS: public AbstractMergeMPS
{
public:
    // Data structures
    enum CandidateVariableType
    {
        CAPACITY,
        DX_PLUS,
        DX_MINUS
    };

    static CandidateVariableType parse_variable_type(const std::string& s);
    static char parse_constraint_type(const std::string& s);

    // This structure contains the position of the variables in the merged problem
    // Its capacity, corresponding dx_plus and dx_minus
    struct VariablePositions
    {
        int capacity{-1};
        int dx_plus{-1};
        int dx_minus{-1};

        int get(CandidateVariableType t) const;
        void set(CandidateVariableType t, int i);
    };

    // candidate_name -> node_name -> variable_positions
    using CandidatesCouplingMap = std::map<std::string, std::map<std::string, VariablePositions>>;

    // Contains the cost data of a candidate
    struct CandidateCosts
    {
        explicit CandidateCosts(const Json::Value& data);
        double operation_maintenance{0.};
        double investment{0.};
        double retirement{0.};

        // Get the cost associated with one of the types of variable
        double get(CandidateVariableType t) const;
    };

    // Reference to a candidate
    using VariableRef = std::tuple<std::string, std::string, CandidateVariableType>;

    // Trajectory constraints
    struct TrajectoryConstraint
    {
        explicit TrajectoryConstraint(const Json::Value& data);

        std::map<VariableRef, double> coefficients_map;
        double rhs{0.};
        char constraint_type;
    };

    struct TrajectoryGlobalData
    {
        TrajectoryGlobalData() = default;
        explicit TrajectoryGlobalData(const Json::Value& data);

        std::map<std::string, double> initial_capacities;

        std::vector<TrajectoryConstraint> trajectory_constraints;
    };

    struct TrajectoryNode
    {
        TrajectoryNode() = default;
        TrajectoryNode(std::string node, const Json::Value& data);

        std::string name;

        std::optional<std::string> parent{std::nullopt};

        // Stores the costs of each candidates' three associated variable at this node.
        std::map<std::string, CandidateCosts> candidates_costs;
    };

    using TrajectoryTree = std::vector<TrajectoryNode>;

    MergeMasterTrajectoryMPS(MergeMPSOptions options,
                             Logger logger,
                             std::shared_ptr<Output::OutputWriter> writer,
                             std::filesystem::path tree_filename,
                             std::filesystem::path annual_lp_filename);

    void launch() override;

private:
    void read_tree_structure_file();
    void read_node_lp_paths();

    void check_nodes_have_lp_folder();

    void build_problem();
    void add_coupling_constraints();
    void add_delta_variables();
    void add_delta_variables_constraints();
    void set_objective_from_data();

    std::string make_prefix_from_node(const std::string& node_name) const;
    double get_candidate_initial_value(const std::string& candidate) const;

    std::filesystem::path tree_path_;
    std::filesystem::path lp_reference_file_filepath_;
    TrajectoryTree tree_;                       // Contains each node's information
    TrajectoryGlobalData trajectory_data_{};    // Contains the global trajectory data
    CandidatesCouplingMap candidates_coupling_; // Links the same candidates in different nodes
    NodesToLpDataLocationMap nodes_lp_info_;    // Info on the lp folder & files for each node
    CouplingMap structure_;
};
