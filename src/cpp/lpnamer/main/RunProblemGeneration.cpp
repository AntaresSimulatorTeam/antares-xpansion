
#include "RunProblemGeneration.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include "ActiveLinks.h"
#include "AdditionalConstraints.h"
#include "Candidate.h"
#include "CandidatesINIReader.h"
#include "Clock.h"
#include "LauncherHelpers.h"
#include "LinkProblemsGenerator.h"
#include "LinkProfileReader.h"
#include "MasterGeneration.h"
#include "MasterProblemBuilder.h"
#include "ProblemGenerationExeOptions.h"
#include "Timer.h"
#include "solver_utils.h"
#include "WeightsFileReader.h"
#include "GeneralDataReader.h"
#include "WeightsFileWriter.h"
#include "MpsTxtWriter.h"
#include "LpFilesExtractor.h"

void ProcessWeights(
    const std::filesystem::path& xpansion_output_dir,
    std::filesystem::path antares_archive_path,
    const std::filesystem::path& weights_file,
    ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger){
    const auto settings_dir = xpansion_output_dir / ".." / ".." / "settings";
    const auto general_data_file = settings_dir/ "generaldata.ini";
    auto genera_data_reader = GeneralDataIniReader(general_data_file, logger);
    auto active_years = genera_data_reader.GetActiveYears();
    WeightsFileReader weights_file_reader(weights_file, active_years.size(), logger) ;
    weights_file_reader.CheckWeightsFile();
    auto weights_vector = weights_file_reader.WeightsList();
    auto yearly_weight_writer = YearlyWeightsWriter(xpansion_output_dir,
             antares_archive_path, weights_vector, weights_file.filename(), active_years);
    yearly_weight_writer.CreateWeightFile();
    }

void WriteMpsTxt(const std::filesystem::path& antares_archive_path, const std::filesystem::path& xpansion_output_dir){
  const auto MPS_TXT = "mps.txt";
  auto mps_txt_writer = MpsTxtWriter(antares_archive_path, xpansion_output_dir/MPS_TXT);
  mps_txt_writer.Write();
  }

  void ExtractUtilsFiles(const std::filesystem::path& antares_archive_path, const std::filesystem::path& xpansion_output_dir,
    ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger)
  {
    auto utils_files_extractor = LpFilesExtractor(antares_archive_path, xpansion_output_dir , logger); 
    utils_files_extractor.ExtractFiles();
  }

void RunProblemGeneration(
    const std::filesystem::path& xpansion_output_dir, const std::string& master_formulation,
    const std::string& additionalConstraintFilename_l,
    std::filesystem::path antares_archive_path,
    ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger,
    const std::filesystem::path& log_file_path, bool zip_mps, const std::filesystem::path& weights_file) {
  (*logger)(ProblemGenerationLog::LOGLEVEL::INFO)
      << "Launching Problem Generation" << std::endl;

  Timer problem_generation_timer;
  if(!weights_file.empty())
  { 
    ProcessWeights(xpansion_output_dir, antares_archive_path, weights_file, logger);   
  }
  
  WriteMpsTxt(antares_archive_path, xpansion_output_dir);
  
  ExtractUtilsFiles(antares_archive_path,  xpansion_output_dir, logger);
  
  ActiveLinksBuilder linkBuilder = get_link_builders(xpansion_output_dir, logger);

  if ((master_formulation != "relaxed") && (master_formulation != "integer")) {
    (*logger)(ProblemGenerationLog::LOGLEVEL
    ::FATAL) << "Invalid formulation argument : argument must be "
                 "\"integer\" or \"relaxed\""
              << std::endl;
    std::exit(1);
  }

  AdditionalConstraints additionalConstraints(logger);
  if (!additionalConstraintFilename_l.empty()) {
    additionalConstraints =
        AdditionalConstraints(additionalConstraintFilename_l, logger);
  }

  Couplings couplings;
  std::string solver_name = "CBC";
  std::vector<ActiveLink> links = linkBuilder.getLinks();
  LinkProblemsGenerator linkProblemsGenerator(links, solver_name, logger,
                                              log_file_path, zip_mps);
  linkProblemsGenerator.treatloop(xpansion_output_dir, antares_archive_path, couplings);

  MasterGeneration master_generation(xpansion_output_dir, links, additionalConstraints,
                                     couplings, master_formulation, solver_name,
                                     logger, log_file_path);
  (*logger)(ProblemGenerationLog::LOGLEVEL::INFO)
      << "Problem Generation ran in: "
      << format_time_str(problem_generation_timer.elapsed()) << std::endl;
}
