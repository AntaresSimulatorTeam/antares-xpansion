
#include <execution>
#include <tbb/tbb.h>

#include "antares-xpansion/lpnamer/main/ProblemGeneration.h"

#include <antares/api/solver.h>

#include <iostream>
#include <utility>

#include "antares-xpansion/lpnamer/input_reader/SettingsReader.h"
#include "Version.h"
#include "antares-xpansion/helpers/Timer.h"
#include "antares-xpansion/lpnamer/helper/ProblemGenerationLogger.h"
#include "antares-xpansion/lpnamer/input_reader/GeneralDataReader.h"
#include "antares-xpansion/lpnamer/input_reader/LpFilesExtractor.h"
#include "antares-xpansion/lpnamer/input_reader/MpsTxtWriter.h"
#include "antares-xpansion/lpnamer/input_reader/SettingsReader.h"
#include "antares-xpansion/lpnamer/input_reader/WeightsFileReader.h"
#include "antares-xpansion/lpnamer/input_reader/WeightsFileWriter.h"
#include "antares-xpansion/lpnamer/model/ActiveLinks.h"
#include "antares-xpansion/lpnamer/problem_modifier/AdditionalConstraints.h"
#include "antares-xpansion/lpnamer/problem_modifier/FileProblemsProviderAdapter.h"
#include "antares-xpansion/lpnamer/problem_modifier/LauncherHelpers.h"
#include "antares-xpansion/lpnamer/problem_modifier/LinkProblemsGenerator.h"
#include "antares-xpansion/lpnamer/problem_modifier/MPSFileWriter.h"
#include "antares-xpansion/lpnamer/problem_modifier/MasterGeneration.h"
#include "antares-xpansion/lpnamer/problem_modifier/MasterProblemBuilder.h"
#include "antares-xpansion/lpnamer/problem_modifier/ProblemVariablesFileAdapter.h"
#include "antares-xpansion/lpnamer/problem_modifier/ProblemVariablesFromProblemAdapter.h"
#include "antares-xpansion/lpnamer/problem_modifier/ProblemVariablesZipAdapter.h"
#include "antares-xpansion/lpnamer/problem_modifier/XpansionProblemsFromAntaresProvider.h"
#include "antares-xpansion/lpnamer/problem_modifier/ZipProblemsProviderAdapter.h"
#include "antares-xpansion/xpansion_interfaces/LogUtils.h"
#include "antares-xpansion/xpansion_interfaces/StringManip.h"
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
    : options_(options) {
  if (!options_.StudyPath().empty()) {
    mode_ = SimulationInputMode::ANTARES_API;
  } else if (!options_.XpansionOutputDir().empty()) {
    mode_ = SimulationInputMode::FILE;
  } else if (!options_.ArchivePath().empty()) {
    mode_ = SimulationInputMode::ARCHIVE;
  }
  if (!mode_) {
    throw LogUtils::XpansionError<std::runtime_error>(
        "SimulationInputMode is unknown", LOGLOCATION);
  }
}

std::filesystem::path ProblemGeneration::performAntaresSimulation() {
  auto results = Antares::API::PerformSimulation(options_.StudyPath());
  // Add parallel

  // Handle errors
  if (results.error) {
    throw LogUtils::XpansionError<std::runtime_error>(
        "Antares simulation failed:\n\t" + results.error->reason, LOGLOCATION);
  }

  lps_ = std::move(results.antares_problems);
  return {results.simulationPath};
}

std::filesystem::path ProblemGeneration::updateProblems() {
  using namespace std::string_literals;
  std::filesystem::path xpansion_output_dir;
  const auto archive_path = options_.ArchivePath();
  std::filesystem::path study_dir;

  if (mode_ == SimulationInputMode::ARCHIVE) {
    xpansion_output_dir =
        options_.deduceXpansionDirIfEmpty(xpansion_output_dir, archive_path);
    study_dir = std::filesystem::absolute(archive_path).parent_path().parent_path();
    //Assume study/output/archive.zip
  }

  if (mode_ == SimulationInputMode::ANTARES_API) {
    simulation_dir_ = performAntaresSimulation();
    study_dir = options_.StudyPath();
  }

  if (mode_ == SimulationInputMode::FILE) {
    simulation_dir_ = options_.XpansionOutputDir();  // Legacy naming.
    // options_.XpansionOutputDir() point in fact to a simulation output from
    // antares
    study_dir = std::filesystem::absolute(simulation_dir_).parent_path().parent_path(); //Assume study/output/simulation
  }

  if (mode_ == SimulationInputMode::ANTARES_API ||
      mode_ == SimulationInputMode::FILE) {
    xpansion_output_dir = simulation_dir_;
  }

  const auto log_file_path =
      xpansion_output_dir / "lp"s / "ProblemGenerationLog.txt"s;

  CreateDirectories(xpansion_output_dir);  // Ca ou -Xpansion ?
  auto logger = ProblemGenerationLog::BuildLogger(log_file_path, std::cout,
                                                  "Problem Generation"s);

  auto master_formulation = options_.MasterFormulation();
  auto additionalConstraintFilename_l =
      options_.AdditionalConstraintsFilename();
  auto weights_file = options_.WeightsFile();
  auto unnamed_problems = options_.UnnamedProblems();

  set_solver(study_dir, logger.get());

  RunProblemGeneration(xpansion_output_dir, master_formulation,
                       additionalConstraintFilename_l, archive_path, logger,
                       log_file_path, weights_file, unnamed_problems);
  return xpansion_output_dir;
}

std::shared_ptr<ArchiveReader> InstantiateZipReader(
    const std::filesystem::path& antares_archive_path);
void ProblemGeneration::ProcessWeights(
    const std::filesystem::path& xpansion_output_dir,
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
  auto yearly_weight_writer =
      YearlyWeightsWriter(xpansion_output_dir, weights_vector,
                          weights_file.filename(), active_years);
  yearly_weight_writer.CreateWeightFile();
}

void ProblemGeneration::ExtractUtilsFiles(
    const std::filesystem::path& antares_archive_path,
    const std::filesystem::path& xpansion_output_dir,
    ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger) {
  auto utils_files_extractor =
      LpFilesExtractor(antares_archive_path, xpansion_output_dir,
                       std::move(logger), mode_.value(), simulation_dir_);
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

/**
 *
 * @param solver_log_manager
 * @param solver_name
 * @param mpsList
 * @param lpDir_
 * @param reader shared pointer to the archive reader to share with
 * ZipProblemsProviderAdapter
 * @param with_archive
 * @param lps data from antares. Passed by reference to prevent heavy copy
 * @return
 */
std::vector<std::shared_ptr<Problem>> ProblemGeneration::getXpansionProblems(
    SolverLogManager& solver_log_manager, const std::string& solver_name,
    const std::vector<ProblemData>& mpsList, std::filesystem::path& lpDir_,
    std::shared_ptr<ArchiveReader> reader,
    const Antares::Solver::LpsFromAntares& lps = {}) {
  std::vector<std::string> problem_names;
  std::transform(mpsList.begin(), mpsList.end(),
                 std::back_inserter(problem_names),
                 [](ProblemData const& data) { return data._problem_mps; });
  switch (mode_.value()) {
    case SimulationInputMode::FILE: {
      FileProblemsProviderAdapter adapter(lpDir_, problem_names);
      return adapter.provideProblems(solver_name, solver_log_manager);
    }
    case SimulationInputMode::ARCHIVE: {
      ZipProblemsProviderAdapter adapter(lpDir_, std::move(reader),
                                         problem_names);
      return adapter.provideProblems(solver_name, solver_log_manager);
    }
    case SimulationInputMode::ANTARES_API: {
      XpansionProblemsFromAntaresProvider adapter(lps);
      return adapter.provideProblems(solver_name, solver_log_manager);
    }
    default:
      throw LogUtils::XpansionError<std::runtime_error>(
          "Unhandled simulation mode", LOGLOCATION);
  }
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

  SolverLoader::GetAvailableSolvers(logger);  // Dirty fix to populate static
                                              // value outside multi thread code
  Timer problem_generation_timer;

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

  auto files_mapper = FilesMapper(antares_archive_path, xpansion_output_dir);
  auto mpsList = files_mapper.MpsAndVariablesFilesVect();

  auto solver_log_manager = SolverLogManager(log_file_path);
  Couplings couplings;
  LinkProblemsGenerator linkProblemsGenerator(
      lpDir_, links, solver_name_, logger, solver_log_manager, rename_problems);
  std::shared_ptr<ArchiveReader> reader =
      antares_archive_path.empty() ? std::make_shared<ArchiveReader>()
                                   : InstantiateZipReader(antares_archive_path);

  /* Main stuff */
  std::vector<std::shared_ptr<Problem>> xpansion_problems = getXpansionProblems(
      solver_log_manager, solver_name_, mpsList, lpDir_, reader, lps_);

  std::vector<std::pair<std::shared_ptr<Problem>, ProblemData>>
      problems_and_data;
  for (int i = 0; i < xpansion_problems.size(); ++i) {
    if (mode_ == SimulationInputMode::ANTARES_API) {
      ProblemData data{xpansion_problems.at(i)->_name, {}};
      problems_and_data.emplace_back(xpansion_problems.at(i), data);
    } else {
      xpansion_problems.at(i)->_name = mpsList.at(i)._problem_mps;
      problems_and_data.emplace_back(xpansion_problems.at(i), mpsList.at(i));
    }
  }
  auto mps_file_writer = std::make_shared<MPSFileWriter>(lpDir_);
  std::for_each(
      std::execution::par, problems_and_data.begin(), problems_and_data.end(),
      [&](const auto& problem_and_data) {
        const auto& [problem, data] = problem_and_data;
        std::shared_ptr<IProblemVariablesProviderPort> variables_provider;
        switch (mode_.value()) {
          case SimulationInputMode::FILE:
            variables_provider = std::make_shared<ProblemVariablesFileAdapter>(
                data, links, logger, lpDir_);
            break;
          case SimulationInputMode::ARCHIVE:
            if (rename_problems) {
              variables_provider = std::make_shared<ProblemVariablesZipAdapter>(
                  reader, data, links, logger);
            } else {
              variables_provider =
                  std::make_shared<ProblemVariablesFromProblemAdapter>(
                      problem, links, logger);
            }
            break;
          case SimulationInputMode::ANTARES_API:
            variables_provider =
                std::make_shared<ProblemVariablesFromProblemAdapter>(
                    problem, links, logger);
            break;
          default:
            (*logger)(LogUtils::LOGLEVEL::ERR) << "Undefined mode";
            break;
        }
        linkProblemsGenerator.treat(data._problem_mps, couplings, problem.get(),
                                    variables_provider.get(),
                                    mps_file_writer.get());
      });

  if (!weights_file.empty()) {
    ProcessWeights(xpansion_output_dir, weights_file, logger);
  }

  if (mode_ == SimulationInputMode::ARCHIVE) {
    reader->Close();
    reader->Delete();
  }
  MasterGeneration master_generation(
      xpansion_output_dir, links, additionalConstraints, couplings,
      master_formulation, solver_name_, logger, solver_log_manager);
  (*logger)(LogUtils::LOGLEVEL::INFO)
      << "Problem Generation ran in: "
      << format_time_str(problem_generation_timer.elapsed()) << std::endl;
}
void ProblemGeneration::set_solver(std::filesystem::path study_dir, ProblemGenerationLog::ProblemGenerationLogger* logger) {
    SettingsReader settingsReader(study_dir / "user" / "expansion" / "settings.ini", logger);
    solver_name_ = settingsReader.Solver();
}

std::shared_ptr<ArchiveReader> InstantiateZipReader(
    const std::filesystem::path& antares_archive_path) {
  auto reader = std::make_shared<ArchiveReader>(antares_archive_path);
  reader->Open();
  return reader;
}
