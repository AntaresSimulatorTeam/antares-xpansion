#pragma once

#include <antares-xpansion/benders/merge_mps/MergeMPS.h>
#include <antares-xpansion/benders/merge_master_mps/MasterCouplingConstants.h>

// Probably needs to be changed
class InvalidMasterStructureFileException: public std::runtime_error
{
public:
    explicit InvalidMasterStructureFileException(const std::string& arg):
        std::runtime_error(arg) {}
};

class MergeMasterTrajectoryMPS : public AbstractMergeMPS
{
public:
    // Data structures

    // This structure contains the position of the variables in the merged problem
    // Its capacity, corresponding dx_plus and dx_minus
    struct VariablePositions{
        int capacity = -1;
        int dx_plus = -1;
        int dx_minus = -1;
    };

    // candidate_name -> node_name -> variable_positions
    // This one we need to store as a map to avoid linear time lookup of parent's position
    typedef std::map<std::string, std::map<std::string, VariablePositions>> CandidatesCouplingMap;

    struct TrajectoryGlobalData
    {
        TrajectoryGlobalData() {}; 
        TrajectoryGlobalData(const Json::Value& data);
        
        std::map<std::string, double> initial_capacities;
    };

    // Will be changed
    // struct TrajectoryConstraints
    // {
    //     double min_investment;
    //     double max_investment;

    //     double min_decommissioning;
    //     double max_decommissioning;
    // };

    struct TrajectoryNode
    {
        TrajectoryNode() {};
        TrajectoryNode(const std::string& node, const Json::Value& data);

        std::string name;
        std::filesystem::path path{};
        std::string master_mps_file;
        std::string structure_file;

        std::string master_name = MasterCouplingConstants::DEFAULT_MASTER_NAME;
        std::optional<std::string> parent{std::nullopt};
        double weight{1.};

        // To be changed, it heavily depends on how we define the trajectory constraints
        //std::map<std::string, TrajectoryConstraints> constraints{};
    };

    using TrajectoryTree = std::vector<TrajectoryNode>;

public:
    MergeMasterTrajectoryMPS(MergeMPSOptions options,
                            Logger logger,
                            std::shared_ptr<Output::OutputWriter> writer,
                            const std::filesystem::path& tree_filename) :
        AbstractMergeMPS(options, logger, writer),
        tree_path_(tree_filename) {};

    // Method
    void launch() override;


private:
    // Initilization : reading the master structure file
    void read_master_structure(const std::filesystem::path& path);
    // Methods specific to this derived class
    void add_delta_variables();
    void add_delta_variables_constraints();
    std::string make_prefix_from_node(const std::string& node_name) const;
    double get_candidate_initial_value(const std::string& candidate) const;

    // Overrides
    void build_problem();
    void add_coupling_constraints();


private : 
    // Attribute
    std::filesystem::path tree_path_;
    TrajectoryTree tree_; // Contains each node's information
    TrajectoryGlobalData trajectory_data_; // Contains the global trajectory data
    CandidatesCouplingMap candidates_coupling_; // Links the same candidates in different nodes
    CouplingMap structure_;
};
