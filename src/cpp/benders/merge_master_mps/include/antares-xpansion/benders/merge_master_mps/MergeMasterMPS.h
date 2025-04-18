#pragma once

#include <antares-xpansion/benders/benders_core/common.h>
#include <antares-xpansion/benders/factories/WriterFactories.h>
#include <antares-xpansion/helpers/solver_utils.h>

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


class MergeMasterMPS
{
private:
    /*!
    *  \brief Add the trajectory constraints defined in each node's data to the merged master problem
    *
    *  \param merged_solver : solver to modify
    *  \param master_coupling : MasterCouplingMap containing the data linking the nodes
    *  \param trajectory_data : TrajectoryGlobalData, contains the initial invested capacities
    *  \param candidates_coupling : CouplingMap that links, for each candidate, for each node in appears in,
    *  the corresponding variable position in the merged problem.
    */
    void addTrajectoryConstraints(
        SolverAbstract& merged_solver,
        const MasterCouplingMap& master_coupling,
        const TrajectoryGlobalData& trajectory_data,
        const CandidatesCouplingMap& candidates_coupling
    );

    /*!
    *  \brief Add the delta variables.
    *   This will modify the candidates_coupling variable such that the
    *   VariablePositions objects now also contain the positions of the dx variables
    *   Note that this function does not add the constraints that link the dx variables with the capacities at each node.
    *
    *  \param merged_solver : solver to modify
    *  \param master_coupling : MasterCouplingMap containing the data linking the nodes
    *  \param trajectory_data : TrajectoryGlobalData, contains the initial invested capacities
    *  \param candidates_coupling : CandidatesCouplingMap that links, for each candidate, for each node in appears in,
    *  the corresponding variables' positions in the merged problem.
    * \param candidates_names : Set that contains each of the names in the candidates.
    */
    void addDeltaVariables(
        SolverAbstract& merged_solver,
        const MasterCouplingMap& master_coupling,
        CandidatesCouplingMap& candidates_coupling,
        const CandidatesNames& candidates_names
    );

        /*!
    *  \brief Add the delta variables constraints that link the capacities in subsequent nodes..
    *
    *  \param merged_solver : solver to modify
    *  \param master_coupling : MasterCouplingMap containing the data linking the nodes
    *  \param trajectory_data : TrajectoryGlobalData, contains the initial invested capacities
    *  \param candidates_coupling : CandidatesCouplingMap that links, for each candidate, for each node in appears in,
    *  the corresponding variables' positions in the merged problem.
    * \param candidates_names : Set that contains each of the names in the candidates.
    */
    void addDeltaVariablesConstraints(
       SolverAbstract& merged_solver,
       const MasterCouplingMap& master_coupling,
       const TrajectoryGlobalData& trajectory_data,
       const CandidatesCouplingMap& candidates_coupling,
       const CandidatesNames& candidates_names
   );


public:
    MergeMasterMPS(MergeMasterMPSOptions options, 
                    Logger logger, 
                    std::shared_ptr<Output::OutputWriter> writer = build_void_writer());

    void launch();

    MergeMasterMPSOptions _options;
    Logger _logger;
    std::shared_ptr<Output::OutputWriter> _writer;
};
