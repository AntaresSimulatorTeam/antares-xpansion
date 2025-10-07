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
#include "antares-xpansion/core/ProblemFormat.h"
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
      ConfigurationManager::ConfigDirectories directories,
      const ReservoirManagement& reservoirManagement,
      Logger logger,
      const std::string& solverName = "xpress",
      unsigned int startWeek = 1,
      unsigned int endWeek = 52,
      bool savePbFiles = false,
      const std::string& problemFormat = "OPTIMIZED");
    virtual ~ProblemGenerationForWaterValueCalculation() = default;
    std::map<Antares::Solver::WeeklyProblemId, std::shared_ptr<Problem>> updateProblems(
      const GridDefinition& gridDefinition);

private:
    std::map<Antares::Solver::WeeklyProblemId, std::shared_ptr<Problem>>
    CleanProblemsForBellmanCalculations(const std::filesystem::path& xpansion_output_dir,
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
    bool writePbFiles;
    ProblemsFormat problemFormat;
    Logger logger;
};
