//
// Created by marechaljas on 02/08/22.
//

#pragma once

#include "SimulationOptions.h"
#include "antares-xpansion/benders/output/OutputWriter.h"
#include "antares-xpansion/xpansion_interfaces/ILogger.h"

namespace Benders
{

class StartUp
{
public:
    bool StudyAlreadyAchievedCriterion(const SimulationOptions& options,
                                       Output::OutputWriter* writer,
                                       const Logger& logger) const;
};

} // namespace Benders
