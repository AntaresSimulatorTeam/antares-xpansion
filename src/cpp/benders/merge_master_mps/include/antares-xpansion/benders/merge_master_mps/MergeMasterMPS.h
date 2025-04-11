#pragma once

#include <antares-xpansion/benders/benders_core/common.h>
#include <antares-xpansion/benders/factories/WriterFactories.h>
#include <antares-xpansion/helpers/solver_utils.h>

#include <antares-xpansion/benders/merge_master_mps/MasterCoupling.h>


typedef BaseOptions MergeMasterMPSOptions;



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
        const CouplingMap& candidates_coupling
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
