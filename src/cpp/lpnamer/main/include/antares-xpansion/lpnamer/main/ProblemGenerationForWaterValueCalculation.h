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
    enum class WaterValueComputationMode
    {
        MULTIVARIATE,
        SEQUENTIAL_UPDATE_TRAJECTORY, // multistock
        SEQUENTIAL_IGNORE_TRAJECTORY, // like multistock but without optimal trajectories
    };
    explicit ProblemGenerationForWaterValueCalculation(
      ConfigurationManager::ConfigDirectories directories,
      // const ReservoirManagement& reservoirManagement,
      Logger logger,
      const std::string& solverName = "xpress",
      unsigned int startWeek = 1,
      unsigned int endWeek = 52,
      bool savePbFiles = false,
      const std::string& problemFormat = "OPTIMIZED",
      const WaterValueComputationMode& computationMode = WaterValueComputationMode::MULTIVARIATE);
    virtual ~ProblemGenerationForWaterValueCalculation() = default;
    std::map<Antares::Solver::WeeklyProblemId, std::shared_ptr<Problem>> updateProblems(
      const GridDefinition& gridDefinition,
      const ReservoirManagement& reservoirManagement,
      const std::string& areaName = "");
    void initializeOptimalTrajectories(std::shared_ptr<GridCollection> gridCollection) const;
    static WaterValueComputationMode getComputationModeFromGrid(
      const GridCollection& grid,
      bool ignoreOptimalTrajectory = false);

    WaterValueComputationMode getComputationMode() const
    {
        return computationMode;
    }

private:
    /// @brief Function that cleans all problems to repare them to compute Bellman values
    /// @param xpansion_output_dir output folder
    /// @param log_file_path path to the log file
    /// @param gridDefinition the full grid definition
    /// @param reservoirManagement an instance of ReservoirManagement holding all reservoirs
    /// @param areaName The optional name of the area, used only in a multistock context
    /// @return
    std::map<Antares::Solver::WeeklyProblemId, std::shared_ptr<Problem>>
    cleanProblemsForBellmanCalculations(const std::filesystem::path& xpansion_output_dir,
                                        const std::filesystem::path& log_file_path,
                                        const GridDefinition& gridDefinition,
                                        const ReservoirManagement& reservoirManagement,
                                        const std::string& areaName);

    void cleanProblemForBellmanCalculations(std::shared_ptr<Problem> problem,
                                            const GridDefinition& gridDefinition,
                                            const ReservoirManagement& reservoirManagement,
                                            const std::string& areaName,
                                            std::string& pbName,
                                            Antares::Solver::WeeklyProblemId pbId);
    void cleanReservoirConstraints(std::shared_ptr<Problem> problem,
                                   const Reservoir& reservoir,
                                   Antares::Solver::WeeklyProblemId pbId);
    void updateReservoirWithOptimalTrajectory(std::shared_ptr<Problem> problem,
                                              const Reservoir& reservoir,
                                              Antares::Solver::WeeklyProblemId pbId);

    void setComputationMode(const WaterValueComputationMode& mode)
    {
        computationMode = mode;
    }

    ConfigurationManager::ConfigDirectories directories;
    std::map<Antares::Solver::WeeklyProblemId, std::shared_ptr<Problem>> problems;
    unsigned int startWeek;
    unsigned int endWeek;
    bool writePbFiles;
    ProblemsFormat problemFormat;
    WaterValueComputationMode computationMode = WaterValueComputationMode::MULTIVARIATE;
    Logger logger;
};
