#pragma once

#include <antares-xpansion/benders/merge_mps/MergeMPS.h>
#include <antares-xpansion/benders/merge_master_mps/MasterCoupling.h>


typedef BaseOptions MergeMasterMPSOptions;


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
    using AbstractMergeMPS::AbstractMergeMPS;


private:
    // Methods specific to this derived class
    void add_delta_variables();
    void add_delta_variables_constraints();
    std::string make_prefix_from_node(const std::string& node_name);
    double get_candidate_initial_value(const std::string& candidate);

    // Overrides
    void build_problem() override;
    void add_coupling_constraints() override;


private : 
    // Attributes
    MasterCouplingMap master_coupling_; // Contains each node's information
    TrajectoryGlobalData trajectory_data_; // Contains the global trajectory data
    CandidatesCouplingMap candidates_coupling_; // Links the same candidates in different nodes
};
