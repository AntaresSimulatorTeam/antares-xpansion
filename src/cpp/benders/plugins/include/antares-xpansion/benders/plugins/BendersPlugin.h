#pragma once

#include "antares-xpansion/benders/benders_core/SubproblemWorker.h"
#include "antares-xpansion/benders/benders_core/common.h"
#include "antares-xpansion/multisolver_interface/SolverAbstract.h"
#include "antares-xpansion/xpansion_interfaces/ILogger.h"

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
    */
    virtual void OnBendersStart() = 0;

    /*
    This method will be called on the end of the benders method
    @inputs :
  */
    virtual void OnBendersEnd() = 0;

    /*
      This method will be called at the beginning of a benders iteration
      @inputs :
    */
    virtual void OnBendersIterationStart() = 0;

    /*
     This method will be called at the end  of a benders iteration
     @inputs :
    */
    virtual void OnBendersIterationEnd() = 0;

    /*
      This method will be called at the start of the master iteration after solving the master
      @inputs :
    */
    virtual void OnBendersMasterResolutionStart() = 0;
    /*
      This method will be called at the end of the master iteration after solving subprolems
      @inputs :

    */
    virtual void OnBendersMasterResolutionEnd() = 0;

    /*
      This method will be called before solving a subproblem (for each subproblem)
      @inputs :
    */
    virtual void OnBendersMicroIterationStart() = 0;

    /*
      This method will be called after solving a subproblem (for each subproblem)
      @inputs :
    */
    virtual void OnBendersMicroIterationEnd() = 0;
};
