#pragma once
#include "antares-xpansion/xpansion_interfaces/ILogger.h"
#include "common.h"

class CouplingMapGenerator
{
public:
    static CouplingMap BuildInput(const std::filesystem::path& structure_path,
                                  ILoggerXpansion* logger,
                                  const std::string& context = "Benders");

    static SubProblemConstraintMap BuildSubProblemConstaintMap(const CouplingMap& coupling_map) ; 
};
