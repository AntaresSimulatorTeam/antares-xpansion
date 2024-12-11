//
// Created by marechaljas on 27/10/23.
//

#pragma once

#include <antares/solver/lps/LpsFromAntares.h>

#include <filesystem>
#include <string>
#include <optional>

#include "antares-xpansion/lpnamer/input_reader/MpsTxtWriter.h"
#include "antares-xpansion/lpnamer/model/Problem.h"
#include "antares-xpansion/lpnamer/model/SimulationInputMode.h"
#include "antares-xpansion/helpers/ArchiveReader.h"
#include "antares-xpansion/lpnamer/main/ProblemGenerationExeOptions.h"
#include "antares-xpansion/lpnamer/helper/ProblemGenerationLogger.h"
#include "ProblemGenerationOptions.h"
#include "antares-xpansion/multisolver_interface/SolverAbstract.h"

class ProblemGeneration {
 public:
  explicit ProblemGeneration(ProblemGenerationOptions& options);
  virtual ~ProblemGeneration() = default;
  std::filesystem::path updateProblems();
  ProblemGenerationOptions& options_;

 private:
  virtual void RunProblemGeneration(
      const std::filesystem::path& xpansion_output_dir,
      const std::string& master_formulation,
      const std::string& additionalConstraintFilename_l,
      const std::filesystem::path& archive_path,
      ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger,
      const std::filesystem::path& log_file_path,
      const std::filesystem::path& weights_file, bool unnamed_problems);

  void ProcessWeights(
      const std::filesystem::path& xpansion_output_dir,
      const std::filesystem::path& weights_file,
      ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger);
  void ExtractUtilsFiles(
      const std::filesystem::path& antares_archive_path,
      const std::filesystem::path& xpansion_output_dir,
      ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger);
  std::vector<std::shared_ptr<Problem>> getXpansionProblems(
      SolverLogManager& solver_log_manager, const std::string& solver_name,
      const std::vector<ProblemData>& mpsList, std::filesystem::path& lpDir_,
      std::shared_ptr<ArchiveReader> reader,
      const Antares::Solver::LpsFromAntares& lps);
  virtual void set_solver(std::filesystem::path study_dir, ProblemGenerationLog::ProblemGenerationLogger* logger);

  Antares::Solver::LpsFromAntares lps_;
  std::optional<SimulationInputMode> mode_;
  virtual std::filesystem::path performAntaresSimulation();
  std::filesystem::path simulation_dir_;
  std::string solver_name_;
};
