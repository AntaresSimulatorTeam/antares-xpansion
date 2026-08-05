#pragma once

#include <memory>
#include <string>

#include "antares-xpansion/benders/benders_core/SubproblemConstraintsManager.h"
#include "antares-xpansion/benders/benders_core/common.h"
#include "antares-xpansion/multisolver_interface/SolverAbstract.h"

/*
  This interface will be implemented each time we need to add call backs.
*/
class BendersPlugin
{
public:
    virtual ~BendersPlugin() = default;

    /*
      This method will be called on the start of the benders method
      @inputs :
            - subproblem_map : map of subproblem workers
            - logger : benders logger
            - options : study configurations
            - solver_log_manager : solver log manager
    */
    virtual void OnBendersStart(const SubproblemsMapPtr& subproblem_map,
                                const Logger& logger,
                                const BendersBaseOptions& options,
                                const SolverLogManager& solver_log_manager,
                                std::shared_ptr<SolverAbstract> sub_problem_solver = nullptr)
      = 0;

    /*
    This method will be called on the end of the benders method
    @inputs :

  */
    virtual void OnBendersEnd() = 0;

    virtual void OnBendersIterationStart() = 0;

    virtual void OnBendersIterationEnd() = 0;

    virtual void OnBendersSubResolutionStart(
      const std::shared_ptr<SubproblemWorker>& sub_worker = nullptr,
      std::string sub_name = "")
      = 0;
    virtual void OnBendersSubResolutionEnd(std::string sub_name, int num_micro_iter) = 0;
    /*
  This method will be called at the start of the master iteration after solving subprolems
  @inputs :

*/
    virtual void OnBendersMasterResolutionStart() = 0;

    /*
      This method will be called at the end of the master iteration after solving the master
      @inputs :
            - master_out : solution of the master problem
            - num_iter : master iteration number

    */
    virtual void OnBendersMasterResolutionEnd(std::map<std::string, double>& master_out,
                                              int& num_iter)
      = 0;

    /*
      This method will be called before solving a subproblem (for each subproblem)
    */
    virtual void OnBendersMicroIterationStart() = 0;

    /*
      This method will be called after solving a subproblem (for each subproblem)
      @inputs :
            - sub_name : subproblem name
            - added_rows : if any rows we have to add to the subproblem worker
            - solve_time : elapsed time to solve the subproblem
            - num_master_iter : master iteration number
            - num_micro_iter : micro iteration number within the current master iteration
    */
    virtual void OnBendersMicroIterationEnd(std::string sub_name,
                                            bool& added_rows,
                                            std::string solve_time,
                                            int num_master_iter,
                                            int num_micro_iter)
      = 0;
};
