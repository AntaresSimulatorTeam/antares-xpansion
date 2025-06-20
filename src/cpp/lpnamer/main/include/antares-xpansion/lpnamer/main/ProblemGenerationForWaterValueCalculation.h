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
    explicit ProblemGenerationForWaterValueCalculation(ProblemGenerationOptions& options);
    void setGridDefinition(std::shared_ptr<GridDefinition> gridDefinition);
    virtual ~ProblemGenerationForWaterValueCalculation() = default;
    std::filesystem::path updateProblems();
    const ProblemGenerationOptions& options_;

private:
    void CleanProblemsForBellmanCalculations(const std::filesystem::path& xpansion_output_dir,
                                             const std::filesystem::path& log_file_path);
    void ExtractUtilsFiles(const std::filesystem::path& antares_archive_path,
                           const std::filesystem::path& xpansion_output_dir,
                           std::shared_ptr<ProblemGenerationLog::ProblemGenerationLogger> logger);
    std::vector<std::shared_ptr<Problem>> getXpansionProblems(
      SolverLogManager& solver_log_manager,
      const std::vector<ProblemData>& mpsList,
      std::filesystem::path& lpDir_,
      std::shared_ptr<ArchiveReader> reader,
      const Antares::Solver::LpsFromAntares& lps);

    Antares::Solver::LpsFromAntares lps_;
    std::optional<SimulationInputMode> mode_;
    virtual void performAntaresSimulation(const std::filesystem::path& output);
    SolverConfig solver_config_{"Coin"};
    ConfigurationManager configuration_manager_;
    ConfigurationManager::ConfigDirectories directories_;
    std::shared_ptr<GridDefinition> gridDefinition;
};
