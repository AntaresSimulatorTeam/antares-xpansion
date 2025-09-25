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

// temporay before refactoring and using the problems in memory
struct UpdateProblemsResult
{
    std::filesystem::path outputPath;
    unsigned int startWeek;
    unsigned int endWeek;
};

class ProblemGenerationForWaterValueCalculation
{
public:
    explicit ProblemGenerationForWaterValueCalculation(
      ConfigurationManager::ConfigDirectories directories,
      const ReservoirManagement& reservoirManagement,
      std::string solverName = "xpress",
      unsigned int startWeek = 1,
      unsigned int endWeek = 52);
    virtual ~ProblemGenerationForWaterValueCalculation() = default;
    UpdateProblemsResult updateProblems(const GridDefinition& gridDefinition);

private:
    std::filesystem::path CleanProblemsForBellmanCalculations(
      const std::filesystem::path& xpansion_output_dir,
      const std::filesystem::path& log_file_path,
      const GridDefinition& gridDefinition);

    void cleanProblemForBellmanCalculations(std::shared_ptr<Problem> problem,
                                            std::string& pbName,
                                            const GridDefinition& gridDefinition,
                                            Antares::Solver::WeeklyProblemId pbId);

    void addReservoirConstraints(std::shared_ptr<Problem> problem,
                                 Antares::Solver::WeeklyProblemId pbId);

    ConfigurationManager::ConfigDirectories directories;
    std::map<Antares::Solver::WeeklyProblemId, std::shared_ptr<Problem>> problems;
    const ReservoirManagement& reservoirManagement;
    unsigned int startWeek;
    unsigned int endWeek;
};
