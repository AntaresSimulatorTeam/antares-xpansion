//
// Created by marechaljas on 27/10/23.
//

#include "include/ProblemGeneration.h"

#include <execution>
#include <iostream>

#include "ActiveLinks.h"
#include "AdditionalConstraints.h"
#include "GeneralDataReader.h"
#include "LauncherHelpers.h"
#include "LinkProblemsGenerator.h"
#include "LogUtils.h"
#include "LpFilesExtractor.h"
#include "MPSFileWriter.h"
#include "MasterGeneration.h"
#include "MasterProblemBuilder.h"
#include "MpsTxtWriter.h"
#include "ProblemGenerationLogger.h"
#include "ProblemVariablesFromProblemAdapter.h"
#include "ProblemVariablesZipAdapter.h"
#include "StringManip.h"
#include "Timer.h"
#include "WeightsFileReader.h"
#include "WeightsFileWriter.h"
#include "ZipProblemsProviderAdapter.h"
#include "config.h"

static const std::string LP_DIRNAME = "lp";

void CreateDirectories(const std::filesystem::path& output_path) {
  if (!std::filesystem::exists(output_path)) {
    std::filesystem::create_directories(output_path);
  }
  auto lp_path = output_path / LP_DIRNAME;
  if (!std::filesystem::exists(lp_path)) {
    std::filesystem::create_directories(lp_path);
  }
}

ProblemGeneration::ProblemGeneration(ProblemGenerationOptions& options)
    : options_(options) {}
std::filesystem::path ProblemGeneration::updateProblems() {
  auto xpansion_output_dir = options_.XpansionOutputDir();
  auto archive_path = options_.ArchivePath();
  const auto log_file_path =
      xpansion_output_dir / "lp" / "ProblemGenerationLog.txt";

  CreateDirectories(xpansion_output_dir);
  auto logger = ProblemGenerationLog::BuildLogger(log_file_path, std::cout,
                                                  "Problem Generation");

  auto master_formulation = options_.MasterFormulation();
  auto additionalConstraintFilename_l =
      options_.AdditionalConstraintsFilename();
  auto weights_file = options_.WeightsFile();
  auto unnamed_problems = options_.UnnamedProblems();
  RunProblemGeneration(xpansion_output_dir, master_formulation,
                       additionalConstraintFilename_l, archive_path, logger,
                       log_file_path, weights_file, unnamed_problems);
  return xpansion_output_dir;
}

struct Version {
  explicit Version(std::string_view version) {
    auto split_version = StringManip::split(StringManip::trim(version), '.');
    major = std::stoi(split_version[0]);
    minor = std::stoi(split_version[1]);
  }

  bool operator<(const Version& another) const {
    if (this->major == another.major) {
      return this->minor < another.minor;
    } else {
      return this->major < another.major;
    }
  }

  bool operator>(const Version& another) const { return !operator<(another); }

  bool operator==(const Version& another) const {
    return (this->major == another.major && this->minor == another.minor);
  }
  bool operator<=(const Version& another) const {
    return (operator<(another) || operator==(another));
  }
  bool operator>=(const Version& another) const {
    return (operator>(another) || operator==(another));
  }

 private:
  int major;
  int minor;
};

std::shared_ptr<ArchiveReader> InstantiateZipReader(
    const std::filesystem::path& antares_archive_path);
void ProblemGeneration::ProcessWeights(
    const std::filesystem::path& xpansion_output_dir,
    const std::filesystem::path& antares_archive_path,
    const std::filesystem::path& weights_file,
    ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger) {
  const auto settings_dir = xpansion_output_dir / ".." / ".." / "settings";
  const auto general_data_file = settings_dir / "generaldata.ini";
  auto genera_data_reader = GeneralDataIniReader(general_data_file, logger);
  auto active_years = genera_data_reader.GetActiveYears();
  WeightsFileReader weights_file_reader(weights_file, active_years.size(),
                                        logger);
  weights_file_reader.CheckWeightsFile();
  auto weights_vector = weights_file_reader.WeightsList();
  auto yearly_weight_writer = YearlyWeightsWriter(
      xpansion_output_dir, antares_archive_path, weights_vector,
      weights_file.filename(), active_years);
  yearly_weight_writer.CreateWeightFile();
}

void ProblemGeneration::ExtractUtilsFiles(
    const std::filesystem::path& antares_archive_path,
    const std::filesystem::path& xpansion_output_dir,
    ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger) {
  auto utils_files_extractor =
      LpFilesExtractor(antares_archive_path, xpansion_output_dir, logger);
  utils_files_extractor.ExtractFiles();
}

std::vector<ActiveLink> getLinks(
    const std::filesystem::path& xpansion_output_dir,
    ProblemGenerationLog::ProblemGenerationLoggerSharedPointer& logger) {
  ActiveLinksBuilder linkBuilder =
      get_link_builders(xpansion_output_dir, logger);
  std::vector<ActiveLink> links = linkBuilder.getLinks();
  return links;
}

/**
 * TODO Move earlier in the process
 * @param master_formulation
 * @param logger
 */
void validateMasterFormulation(
    const std::string& master_formulation,
    const ProblemGenerationLog::ProblemGenerationLoggerSharedPointer& logger) {
  if ((master_formulation != "relaxed") && (master_formulation != "integer")) {
    (*logger)(LogUtils::LOGLEVEL::FATAL)
        << LOGLOCATION
        << "Invalid formulation argument : argument must be "
           "\"integer\" or \"relaxed\""
        << std::endl;
    exit(1);
  }
}

std::vector<std::shared_ptr<Problem>> getXpansionProblems(
    SolverLogManager& solver_log_manager, const std::string& solver_name,
    const std::vector<ProblemData>& mpsList, std::filesystem::path& lpDir_,
    std::shared_ptr<ArchiveReader>& reader) {
  std::vector<std::string> problem_names;
  std::transform(mpsList.begin(), mpsList.end(),
                 std::back_inserter(problem_names),
                 [](ProblemData const& data) { return data._problem_mps; });
  auto adapter = std::make_shared<ZipProblemsProviderAdapter>(lpDir_, reader,
                                                              problem_names);
  return adapter->provideProblems(solver_name, solver_log_manager);
}

void ProblemGeneration::RunProblemGeneration(
    const std::filesystem::path& xpansion_output_dir,
    const std::string& master_formulation,
    const std::string& additionalConstraintFilename_l,
    const std::filesystem::path& antares_archive_path,
    ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger,
    const std::filesystem::path& log_file_path,
    const std::filesystem::path& weights_file, bool unnamed_problems) {
  (*logger)(LogUtils::LOGLEVEL::INFO)
      << "Launching Problem Generation" << std::endl;
  validateMasterFormulation(master_formulation, logger);
  std::string solver_name = "CBC";  // TODO Use solver selected by user

  Timer problem_generation_timer;
  if (!weights_file.empty()) {
    ProcessWeights(xpansion_output_dir, antares_archive_path, weights_file,
                   logger);
  }

  ExtractUtilsFiles(antares_archive_path, xpansion_output_dir, logger);

  std::vector<ActiveLink> links = getLinks(xpansion_output_dir, logger);

  AdditionalConstraints additionalConstraints(logger);
  if (!additionalConstraintFilename_l.empty()) {
    additionalConstraints =
        AdditionalConstraints(additionalConstraintFilename_l, logger);
  }

  auto lpDir_ = xpansion_output_dir / "lp";
  Version antares_version(ANTARES_VERSION);
  // TODO update the version of simulator that come with named mps
  Version first_version_without_variables_files("8.7");
  auto rename_problems =
      unnamed_problems ||
      antares_version < first_version_without_variables_files;
  (*logger)(LogUtils::LOGLEVEL::INFO)
      << "rename problems: " << std::boolalpha << rename_problems << std::endl;
  auto files_mapper = FilesMapper(antares_archive_path);
  auto mpsList = files_mapper.MpsAndVariablesFilesVect();

  auto solver_log_manager = SolverLogManager(log_file_path);
  Couplings couplings;
  LinkProblemsGenerator linkProblemsGenerator(
      lpDir_, links, solver_name, logger, solver_log_manager, rename_problems);
  std::shared_ptr<ArchiveReader> reader =
      InstantiateZipReader(antares_archive_path);

  /* Main stuff */
  std::vector<std::shared_ptr<Problem>> xpansion_problems = getXpansionProblems(
      solver_log_manager, solver_name, mpsList, lpDir_, reader);

  std::vector<std::pair<std::shared_ptr<Problem>, ProblemData>>
      problems_and_data;
  for (int i = 0; i < xpansion_problems.size(); ++i) {
    xpansion_problems.at(i)->_name = mpsList.at(i)._problem_mps;
    problems_and_data.emplace_back(xpansion_problems.at(i), mpsList.at(i));
  }
  auto mps_file_writer = std::make_shared<MPSFileWriter>(lpDir_);
  std::for_each(
      std::execution::par, problems_and_data.begin(), problems_and_data.end(),
      [&](const auto& problem_and_data) {
        const auto& [problem, data] = problem_and_data;
        std::shared_ptr<IProblemVariablesProviderPort> variables_provider;
        if (rename_problems) {
          variables_provider = std::make_shared<ProblemVariablesZipAdapter>(
              reader, data, links, logger);
        } else {
          variables_provider =
              std::make_shared<ProblemVariablesFromProblemAdapter>(
                  problem, links, logger);
        }
        linkProblemsGenerator.treat(data._problem_mps, couplings, problem.get(),
                                    variables_provider.get(),
                                    mps_file_writer.get());
      });

  reader->Close();
  reader->Delete();

  MasterGeneration master_generation(
      xpansion_output_dir, links, additionalConstraints, couplings,
      master_formulation, solver_name, logger, solver_log_manager);
  (*logger)(LogUtils::LOGLEVEL::INFO)
      << "Problem Generation ran in: "
      << format_time_str(problem_generation_timer.elapsed()) << std::endl;
}
std::shared_ptr<ArchiveReader> InstantiateZipReader(
    const std::filesystem::path& antares_archive_path) {
  auto reader = std::make_shared<ArchiveReader>(antares_archive_path);
  reader->Open();
  return reader;
}
