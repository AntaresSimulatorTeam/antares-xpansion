#pragma once

#include <antares-xpansion/benders/merge_master_mps/MasterStructureKeys.h>
#include <antares-xpansion/benders/merge_mps/MergeMPS.h>

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
        CAPA,
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
    typedef std::map<std::string, std::map<std::string, VariablePositions>> CandidatesCouplingMap;

    // Contains the cost data of a candidates type
    struct CandidateTypeCosts
    {
        CandidateTypeCosts(const Json::Value& data);
        double operation_maintenace{0.};
        double investment{0.};
        double retirement{0.};
        // Get the cost associated with one of the types of variable
        double get(CandidateVariableType t) const;
    };

    // Reference to a candidate
    typedef std::tuple<std::string, std::string, CandidateVariableType> VariableRef;

    // Trajectory constraints
    struct TrajectoryConstraint
    {
        TrajectoryConstraint(const Json::Value& data);

        std::map<VariableRef, double> coefficients_map;
        double rhs{0.};
        char constraint_type;
    };

    struct TrajectoryGlobalData
    {
        TrajectoryGlobalData()
        {
        }

        TrajectoryGlobalData(const Json::Value& data);

        std::map<std::string, double> initial_capacities;

        std::map<std::string, CandidateTypeCosts> candidates_types_costs;
        std::vector<TrajectoryConstraint> trajectory_constraints;
    };

    struct TrajectoryNode
    {
        TrajectoryNode()
        {
        }

        TrajectoryNode(const std::string& node, const Json::Value& data);

        std::string name;
        std::filesystem::path path{};
        std::string master_mps_file;
        std::string structure_file;

        std::string master_name = MasterStructureKeys::DEFAULT_MASTER_NAME;
        std::optional<std::string> parent{std::nullopt};
        double weight{1.};

        // Points from each candidate to the specific costs types associated
        // e.g : "semibase_fr00" -> "ocgt_new_generic"
        // Perhaps could be a map of references to CandidatesCosts objects stored in the global data
        // ?
        std::map<std::string, std::string> candidates_costs_types;
    };

    using TrajectoryTree = std::vector<TrajectoryNode>;

public:
    MergeMasterTrajectoryMPS(MergeMPSOptions options,
                             Logger logger,
                             std::shared_ptr<Output::OutputWriter> writer,
                             const std::filesystem::path& tree_filename):
        AbstractMergeMPS(options, logger, writer),
        tree_path_(tree_filename)
    {
    }

    // Method
    void launch() override;

private:
    // Initilization : reading the master structure file
    void read_tree_structure_file();
    // Methods specific to this derived class
    void add_delta_variables();
    void add_delta_variables_constraints();
    void set_objective_from_data();
    // Getters & utils
    const CandidateTypeCosts& get_candidates_costs(const TrajectoryNode& node,
                                                   const std::string& candidate) const;
    std::string make_prefix_from_node(const std::string& node_name) const;
    double get_candidate_initial_value(const std::string& candidate) const;

    // Overrides
    void build_problem();
    void add_coupling_constraints();

private:
    // Attribute
    std::filesystem::path tree_path_;
    TrajectoryTree tree_;                       // Contains each node's information
    TrajectoryGlobalData trajectory_data_;      // Contains the global trajectory data
    CandidatesCouplingMap candidates_coupling_; // Links the same candidates in different nodes
    CouplingMap structure_;
};
