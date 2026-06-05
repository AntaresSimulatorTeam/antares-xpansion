//
// Created by marechaljas on 27/10/23.
//

#pragma once

#include <filesystem>
#include <optional>
#include <string>

#include <antares/solver/lps/LpsFromAntares.h>

#include "ConfigurationManager.h"
#include "antares-xpansion/bellman_values/ProblemManager.h"
#include "antares-xpansion/evaluator/GridCollection.h"
#include "antares-xpansion/lpnamer/main/ProblemGenerationOptimSimu.h"
#include "antares-xpansion/lpnamer/model/Problem.h"

/// @brief Class to generate and modify problems in memory
class ProblemGenerationForWaterValueCalculation: public ProblemGenerationOptimSimu
{
public:
    enum class WaterValueComputationMode
    {
        SEQUENTIAL_UPDATE_TRAJECTORY, // default multistock-ready approach
        SEQUENTIAL_IGNORE_TRAJECTORY, // without optimal trajectories
    };
    explicit ProblemGenerationForWaterValueCalculation(
      ConfigurationManager::ConfigDirectories directories,
      Logger logger,
      std::shared_ptr<ProblemManager> problemManager,
      const WaterValueComputationMode&
        computationMode = WaterValueComputationMode::SEQUENTIAL_IGNORE_TRAJECTORY,
      unsigned int startWeek = 1,
      unsigned int endWeek = 52);
    virtual ~ProblemGenerationForWaterValueCalculation() = default;
    // std::map<Antares::Solver::WeeklyProblemId, std::shared_ptr<Problem>>
    std::shared_ptr<ProblemManager> updateProblems(
      const GridDefinition& gridDefinition,
      const std::optional<std::string>& areaName = std::nullopt);
    void initializeOptimalTrajectories(std::shared_ptr<GridCollection> gridCollection) const;
    static WaterValueComputationMode getComputationModeFromGrid(bool useOptimalTrajectory = false);

    WaterValueComputationMode getComputationMode() const
    {
        return computationMode;
    }

private:
    struct AffectedColsAndRows /// Collect indices to delete and bounds to change
    {
        std::unordered_map<std::string, int> colNameToIndex;
        std::unordered_map<std::string, int> rowNameToIndex;
        std::vector<int> colsToDelete;
        std::vector<int> rowsToDelete;
        std::vector<int> hydroProdCols;
        std::vector<double> hydroProdBounds;
    };

    /// @brief Function that cleans all problems to repare them to compute Bellman values
    /// @param xpansion_output_dir output folder
    /// @param log_file_path path to the log file
    /// @param gridDefinition the full grid definition
    /// @param reservoirManagement an instance of ReservoirManagement holding all reservoirs
    /// @param areaName The optional name of the area, used only in a multistock context
    /// @return
    // std::map<Antares::Solver::WeeklyProblemId, std::shared_ptr<Problem>>
    std::shared_ptr<ProblemManager> cleanProblemsForBellmanCalculations(
      const std::filesystem::path& xpansion_output_dir,
      const std::filesystem::path& log_file_path,
      const GridDefinition& gridDefinition,
      const std::string& areaName);

    void cleanProblemForBellmanCalculations(std::shared_ptr<Problem> problem,
                                            const GridDefinition& gridDefinition,
                                            const std::string& areaName,
                                            std::string& pbName,
                                            Antares::Solver::WeeklyProblemId pbId);
    void cleanReservoirConstraints(std::shared_ptr<Problem> problem,
                                   const Reservoir& reservoir,
                                   Antares::Solver::WeeklyProblemId pbId,
                                   AffectedColsAndRows& affectedColsAndRows);
    void updateReservoirWithOptimalTrajectory(std::shared_ptr<Problem> problem,
                                              const Reservoir& reservoir,
                                              Antares::Solver::WeeklyProblemId pbId);

    void setComputationMode(const WaterValueComputationMode& mode)
    {
        computationMode = mode;
    }

    WaterValueComputationMode computationMode = WaterValueComputationMode::
      SEQUENTIAL_IGNORE_TRAJECTORY; /// Computation mode for water values
};
