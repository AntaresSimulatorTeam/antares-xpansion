#pragma once

#include <map>
#include <memory>
#include <string>

#include "antares-xpansion/benders/benders_core/ConstraintsReader.h"
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
    */
    virtual void OnBendersStart() = 0;

    /*
    This method will be called on the end of the benders method
  */
    virtual void OnBendersEnd() = 0;

    /*
      This method will be called at the start of the master iteration after solving the master
    */
    virtual void OnBendersMasterIterationStart() = 0;
    /*
      This method will be called at the end of the master iteration after solving subprolems
    */
    virtual void OnBendersMasterIterationEnd() = 0;

    /*
      This method will be called before solving a subproblem (for each subproblem)
    */
    virtual void OnBendersMicroIterationStart() = 0;

    /*
      This method will be called after solving a subproblem (for each subproblem)
    */
    virtual void OnBendersMicroIterationEnd() = 0;
};
