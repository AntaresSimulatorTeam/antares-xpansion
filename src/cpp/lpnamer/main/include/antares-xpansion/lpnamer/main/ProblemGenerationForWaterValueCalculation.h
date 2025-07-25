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
#include "antares-xpansion/grid_evaluator/GridCollection.h"
#include "antares-xpansion/helpers/ArchiveReader.h"
#include "antares-xpansion/lpnamer/helper/ProblemGenerationLogger.h"
#include "antares-xpansion/lpnamer/input_reader/MpsTxtWriter.h"
#include "antares-xpansion/lpnamer/main/ProblemGenerationExeOptions.h"
#include "antares-xpansion/lpnamer/model/Problem.h"
#include "antares-xpansion/lpnamer/model/SimulationInputMode.h"
#include "antares-xpansion/multisolver_interface/SolverAbstract.h"
#include "antares-xpansion/multisolver_interface/SolverConfig.h"

class ProblemGenerationForWaterValueCalculation
{
public:
    explicit ProblemGenerationForWaterValueCalculation(
      ConfigurationManager::ConfigDirectories directories);
    virtual ~ProblemGenerationForWaterValueCalculation() = default;
    std::filesystem::path updateProblems(const GridDefinition& gridDefinition);

private:
    std::filesystem::path CleanProblemsForBellmanCalculations(
      const std::filesystem::path& xpansion_output_dir,
      const std::filesystem::path& log_file_path,
      const GridDefinition& gridDefinition);

    ConfigurationManager::ConfigDirectories directories;
    std::map<Antares::Solver::WeeklyProblemId, std::shared_ptr<Problem>> problems;
};
