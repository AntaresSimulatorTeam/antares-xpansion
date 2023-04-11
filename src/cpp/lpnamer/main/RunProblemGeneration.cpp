
#include "RunProblemGeneration.h"

#include <boost/archive/text_iarchive.hpp>
#include <execution>
#include <iostream>

#include "ActiveLinks.h"
#include "AdditionalConstraints.h"
#include "Clock.h"
#include "GeneralDataReader.h"
#include "LauncherHelpers.h"
#include "LinkProblemsGenerator.h"
#include "LpFilesExtractor.h"
#include "MPSFileWriter.h"
#include "MasterGeneration.h"
#include "MasterProblemBuilder.h"
#include "MpsTxtWriter.h"
#include "ProblemGenerationExeOptions.h"
#include "ProblemVariablesFromProblemAdapter.h"
#include "ProblemVariablesZipAdapter.h"
#include "Timer.h"
#include "WeightsFileReader.h"
#include "WeightsFileWriter.h"
#include "XpansionProblemsFromAntaresProvider.h"
#include "ZipProblemsProviderAdapter.h"
#include "common_lpnamer.h"

std::shared_ptr<ArchiveReader> InstantiateZipReader(
    const std::filesystem::path& antares_archive_path);
void ProcessWeights(
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

void ExtractUtilsFiles(
    const std::filesystem::path& antares_archive_path,
    const std::filesystem::path& xpansion_output_dir,
    ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger) {
  auto utils_files_extractor =
      LpFilesExtractor(antares_archive_path, xpansion_output_dir, logger);
  utils_files_extractor.ExtractFiles();
}

void RunProblemGeneration(
    const std::filesystem::path& xpansion_output_dir,
    const std::string& master_formulation,
    const std::string& additionalConstraintFilename_l,
    const std::filesystem::path& antares_archive_path,
    ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger,
    const std::filesystem::path& log_file_path,
    const std::filesystem::path& weights_file, bool with_variables_files) {
  (*logger)(ProblemGenerationLog::LOGLEVEL::INFO)
      << "Launching Problem Generation" << std::endl;

  Timer problem_generation_timer;
  if (!weights_file.empty()) {
    ProcessWeights(xpansion_output_dir, antares_archive_path, weights_file,
                   logger);
  }

  ExtractUtilsFiles(antares_archive_path, xpansion_output_dir, logger);

  ActiveLinksBuilder linkBuilder =
      get_link_builders(xpansion_output_dir, logger);

  if ((master_formulation != "relaxed") && (master_formulation != "integer")) {
    (*logger)(ProblemGenerationLog::LOGLEVEL ::FATAL)
        << "Invalid formulation argument : argument must be "
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

  auto const mps_file_name = xpansion_output_dir / common_lpnamer::MPS_TXT;

  auto lpDir_ = xpansion_output_dir / "lp";
  LinkProblemsGenerator linkProblemsGenerator(lpDir_, links, solver_name,
                                              logger, log_file_path);
  auto files_mapper = FilesMapper(antares_archive_path);
  auto mpsList = files_mapper.MpsAndVariablesFilesVect();

  bool use_zip_implementation = true;
  bool use_file_implementation = false;

  if (use_zip_implementation) {
    std::shared_ptr<ArchiveReader> reader =
        InstantiateZipReader(antares_archive_path);

    /* Main stuff */
    std::vector<std::string> problem_names;
    std::transform(mpsList.begin(), mpsList.end(),
                   std::back_inserter(problem_names),
                   [](ProblemData const& data) { return data._problem_mps; });
    auto adapter = std::make_shared<ZipProblemsProviderAdapter>(lpDir_, reader,
                                                                problem_names);
    std::vector<std::shared_ptr<Problem>> xpansion_problems =
        adapter->provideProblems(solver_name, log_file_path);

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
          if (with_variables_files) {
            variables_provider = std::make_shared<ProblemVariablesZipAdapter>(
                reader, data, links, logger);
          } else {
            variables_provider =
                std::make_shared<ProblemVariablesFromProblemAdapter>(
                    problem, links, logger);
          }
          linkProblemsGenerator.treat(data._problem_mps, couplings, problem,
                                      variables_provider, mps_file_writer);
        });

    reader->Close();
    reader->Delete();
  } else if (use_file_implementation) {
    /* Main stuff */
    auto mps_file_writer = std::make_shared<MPSFileWriter>(lpDir_);
    linkProblemsGenerator.treatloop(xpansion_output_dir, couplings, mpsList,
                                    mps_file_writer, with_variables_files);

  } else {
    std::filesystem::path path =
        xpansion_output_dir.parent_path().parent_path() /
        "fichierDeSerialisation";
    std::ifstream ifs(xpansion_output_dir.parent_path().parent_path() /
                      "fichierDeSerialisation");
    boost::archive::text_iarchive ia(ifs);

    LpsFromAntares lps;
    ia >> lps;
    lps._constant->Mdeb.push_back(lps._constant->NombreDeCoefficients);

    XpansionProblemsFromAntaresProvider adapter(lps);
    auto xpansion_problems =
        adapter.provideProblems(solver_name, log_file_path);
    std::vector<std::pair<std::shared_ptr<Problem>, ProblemData>>
        problems_and_data;
    for (int i = 0; i < xpansion_problems.size(); ++i) {
      xpansion_problems.at(i)->_name = mpsList.at(i)._problem_mps;
      problems_and_data.emplace_back(xpansion_problems.at(i), mpsList.at(i));
    }

    auto reader = InstantiateZipReader(antares_archive_path);
    auto mps_file_writer = std::make_shared<MPSFileWriter>(lpDir_);

    std::for_each(
        std::execution::par, problems_and_data.begin(), problems_and_data.end(),
        [&](const auto& problem_and_data) {
          const auto& [problem, data] = problem_and_data;
          std::shared_ptr<IProblemVariablesProviderPort> variables_provider;
          if (with_variables_files) {
            variables_provider = std::make_shared<ProblemVariablesZipAdapter>(
                reader, data, links, logger);
          } else {
            variables_provider =
                std::make_shared<ProblemVariablesFromProblemAdapter>(
                    problem, links, logger);
          }
          linkProblemsGenerator.treat(data._problem_mps, couplings, problem,
                                      variables_provider, mps_file_writer);
        });
  }

  MasterGeneration master_generation(
      xpansion_output_dir, links, additionalConstraints, couplings,
      master_formulation, solver_name, logger, log_file_path);
  (*logger)(ProblemGenerationLog::LOGLEVEL::INFO)
      << "Problem Generation ran in: "
      << format_time_str(problem_generation_timer.elapsed()) << std::endl;
}
std::shared_ptr<ArchiveReader> InstantiateZipReader(
    const std::filesystem::path& antares_archive_path) {
  auto reader = std::make_shared<ArchiveReader>(antares_archive_path);
  reader->Open();
  return reader;
}
