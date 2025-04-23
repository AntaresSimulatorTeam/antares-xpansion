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

// This structure contains the position of the variables in the merged problem
// Its capacity, corresponding dx_plus and dx_minus
struct VariablePositions{
    int capacity = -1;
    int dx_plus = -1;
    int dx_minus = -1;
};

// candidate_name -> node_name -> variable_positions
typedef std::map<std::string, std::map<std::string, VariablePositions>> CandidatesCouplingMap;

// Contains the names of the candidates, should be sufficient as we want them to be the same everywhere.
typedef std::set<std::string> CandidatesNames;


class MergeMasterTrajectoryMPS : public AbstractMergeMPS
{

public:
    // Data
    using AbstractMergeMPS::AbstractMergeMPS;

    struct TrajectoryGlobalData
    {
        // I need to define this because I don't like reading the data during the object's construction
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

    using TrajectoryTree = std::map<std::string, TrajectoryNode>;

public:
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
    void build_problem() override;
    void add_coupling_constraints() override;


private : 
    // Attributes
    TrajectoryTree master_coupling_; // Contains each node's information
    TrajectoryGlobalData trajectory_data_; // Contains the global trajectory data
    CandidatesCouplingMap candidates_coupling_; // Links the same candidates in different nodes
    CouplingMap structure_;
};
