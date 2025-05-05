//
// Created by marechaljas on 27/10/23.
//

#pragma once

#include <antares-xpansion/lpnamer/model/ActiveLinks.h>
#include <filesystem>
#include <optional>
#include <string>

#include <antares/solver/lps/LpsFromAntares.h>

#include "ConfigurationManager.h"
#include "ProblemGenerationOptions.h"
#include "antares-xpansion/helpers/ArchiveReader.h"
#include "antares-xpansion/helpers/Timer.h"
#include "antares-xpansion/lpnamer/helper/ProblemGenerationLogger.h"
#include "antares-xpansion/lpnamer/input_reader/MpsTxtWriter.h"
#include "antares-xpansion/lpnamer/main/ProblemGenerationExeOptions.h"
#include "antares-xpansion/lpnamer/model/Problem.h"
#include "antares-xpansion/lpnamer/model/SimulationInputMode.h"
#include "antares-xpansion/lpnamer/problem_modifier/LinkProblemsGenerator.h"
#include "antares-xpansion/multisolver_interface/SolverAbstract.h"
#include "antares-xpansion/multisolver_interface/SolverConfig.h"

class ProblemGeneration
{
public:
    explicit ProblemGeneration(ProblemGenerationOptions& options);
    virtual ~ProblemGeneration() = default;
    std::filesystem::path updateProblems();
    const ProblemGenerationOptions& options_;

private:
    virtual void RunProblemGeneration(
      const std::filesystem::path& xpansion_output_dir,
      const std::string& master_formulation,
      const std::string& additionalConstraintFilename_l,
      const std::filesystem::path& archive_path,
      std::shared_ptr<ProblemGenerationLog::ProblemGenerationLogger> logger,
      const std::filesystem::path& log_file_path,
      const std::filesystem::path& weights_file,
      bool unnamed_problems);

    void ExtractUtilsFiles(const std::filesystem::path& antares_archive_path,
                           const std::filesystem::path& xpansion_output_dir,
                           std::shared_ptr<ProblemGenerationLog::ProblemGenerationLogger> logger);
    std::vector<std::shared_ptr<Problem>> getXpansionProblems(
      SolverLogManager& solver_log_manager,
      const std::vector<ProblemData>& mpsList,
      std::filesystem::path& lpDir_,
      std::shared_ptr<ArchiveReader> reader,
      const Antares::Solver::LpsFromAntares& lps);
    virtual void set_solver(std::filesystem::path study_dir,
                            ProblemGenerationLog::ProblemGenerationLogger* logger);

    Antares::Solver::LpsFromAntares new_lps_;
    std::optional<SimulationInputMode> mode_;
    void onNewLP(Antares::Solver::LpsFromAntares& lps);
    virtual void performAntaresSimulation(const std::filesystem::path& output);
    SolverConfig solver_config_{"Coin"};
    ConfigurationManager configuration_manager_;
    ConfigurationManager::ConfigDirectories directories_;
    std::shared_ptr<ProblemGenerationLog::ProblemGenerationLogger> logger_;
    bool once{false};
    std::vector<ActiveLink> links_;
    std::unique_ptr<SolverLogManager> solver_log_manager_;
    Timer problem_generation_timer;
    Couplings couplings;
    std::vector<std::pair<int, ProblemData>> year_and_data;
    std::unique_ptr<LinkProblemsGenerator> linkProblemsGenerator;
};
