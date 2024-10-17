//
// Created by marechaljas on 02/08/22.
//

#pragma once

#include "antares-xpansion/xpansion_interfaces/ILogger.h"
#include "antares-xpansion/xpansion_interfaces/OutputWriter.h"
#include "SimulationOptions.h"
namespace Benders {

class StartUp {
 public:
  bool StudyAlreadyAchievedCriterion(const SimulationOptions& options,
                                     const Writer& writer,
                                     const Logger& logger) const;
};

}  // namespace Benders
