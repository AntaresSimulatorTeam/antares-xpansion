//
// Created by marechaljas on 27/10/23.
//

#pragma once

#include <filesystem>
#include <optional>
#include <string>

#include <antares/solver/lps/LpsFromAntares.h>

#include "ConfigurationManager.h"
#include "ProblemGenerationOptions.h"
#include "antares-xpansion/bellman_values/ProblemManager.h"
#include "antares-xpansion/core/ProblemFormat.h"
#include "antares-xpansion/helpers/ArchiveReader.h"
#include "antares-xpansion/lpnamer/helper/ProblemGenerationLogger.h"
#include "antares-xpansion/lpnamer/input_reader/MpsTxtWriter.h"
#include "antares-xpansion/lpnamer/main/ProblemGenerationExeOptions.h"
#include "antares-xpansion/lpnamer/model/Problem.h"
#include "antares-xpansion/lpnamer/model/SimulationInputMode.h"
#include "antares-xpansion/multisolver_interface/SolverAbstract.h"
#include "antares-xpansion/multisolver_interface/SolverConfig.h"

/// @brief Class to generate and modify problems in memory
class ProblemGenerationOptimSimu
{
public:
    explicit ProblemGenerationOptimSimu(ConfigurationManager::ConfigDirectories directories,
                                        Logger logger,
                                        std::shared_ptr<ProblemManager> problemManager,
                                        unsigned int startWeek = 1,
                                        unsigned int endWeek = 52);
    virtual ~ProblemGenerationOptimSimu() = default;
    ConfigurationManager::ConfigDirectories
      directories;          /// Directories, used for the original problems generation
    unsigned int startWeek; /// Start week of the problems to take into account
    unsigned int endWeek;   /// End week of the problems to take into account
    std::shared_ptr<ProblemManager> problemManager; /// The manager taking care of reading problems
                                                    /// from disk, problem formats, etc.
    Logger logger;                                  /// Logger used
};
