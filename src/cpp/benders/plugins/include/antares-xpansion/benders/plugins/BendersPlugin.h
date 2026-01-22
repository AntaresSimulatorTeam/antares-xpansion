#pragma once

#include <map>
#include <memory>
#include <string>

#include "antares-xpansion/benders/benders_core/ConstraintsReader.h"
#include "antares-xpansion/benders/benders_core/common.h"
#include "antares-xpansion/multisolver_interface/SolverAbstract.h"

class BendersPlugin
{
public:
    virtual ~BendersPlugin() = default;
    virtual void OnBendersStart(const SubproblemsMapPtr& subproblem_map,
                                const Logger& logger,
                                const BendersBaseOptions& options,
                                const SolverLogManager& solver_log_manager)
      = 0;
    virtual void OnBendersEnd() = 0;
    virtual void OnBendersMasterIterationStart(std::map<std::string, double>&,int&) = 0;
    virtual void OnBendersMasterIterationEnd() = 0;
    virtual void OnBendersMicroIterationStart() = 0;
    virtual void OnBendersMicroIterationEnd(std::string sub_name, bool& added_rows,std::string solve_time) = 0;
};
