
#include "RunProblemGeneration.h"

#include <boost/archive/text_iarchive.hpp>
#include <execution>
#include <iostream>

#include "ActiveLinks.h"
#include "AdditionalConstraints.h"
#include "ArchiveProblemWriter.h"
#include "Clock.h"
#include "GeneralDataReader.h"
#include "LauncherHelpers.h"
#include "LinkProblemsGenerator.h"
#include "LpFilesExtractor.h"
#include "MasterGeneration.h"
#include "MasterProblemBuilder.h"
#include "MpsTxtWriter.h"
#include "ProblemGenerationExeOptions.h"
#include "ProblemVariablesZipAdapter.h"
#include "Timer.h"
#include "WeightsFileReader.h"
#include "WeightsFileWriter.h"
#include "XpansionProblemsFromAntaresProvider.h"
#include "ZipProblemsProviderAdapter.h"
#include "common_lpnamer.h"

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
    const std::filesystem::path& log_file_path, bool zip_mps,
    const std::filesystem::path& weights_file) {
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

  LinkProblemsGenerator linkProblemsGenerator(links, solver_name, logger,
                                              log_file_path, zip_mps);
  auto mpsList = linkProblemsGenerator.readMPSList(mps_file_name);

  bool use_zip_implementation = true;
  bool use_file_implementation = false;
  if (use_zip_implementation) {
    /* Instantiate Zip reader */
    auto reader = std::make_shared<ArchiveReader>(antares_archive_path);
    reader->Open();

    /*Instantiate zip writer */
    auto lpDir_ = xpansion_output_dir / "lp";
    const auto tmpArchiveName = MPS_ZIP_FILE + "-tmp" + ZIP_EXT;
    const auto tmpArchivePath = lpDir_ / tmpArchiveName;
    auto writer = std::make_shared<ArchiveWriter>(tmpArchivePath);
    writer->Open();

    /* Zip problem writer */
    auto problem_writer =
        std::make_shared<ArchiveProblemWriter>(xpansion_output_dir, writer);

    /* Main stuff */
    std::vector<std::string> problem_names;
    std::transform(mpsList.begin(), mpsList.end(),
                   std::back_inserter(problem_names),
                   [](ProblemData const& data) { return data._problem_mps; });
    auto adapter = std::make_shared<ZipProblemsProviderAdapter>(lpDir_, reader,
                                                                problem_names);
    std::vector<std::shared_ptr<Problem>> xpansion_problems =
        adapter->provideProblems(solver_name);

    std::vector<std::pair<std::shared_ptr<Problem>, ProblemData>>
        problems_and_data;
    for (int i = 0; i < xpansion_problems.size(); ++i) {
      xpansion_problems.at(i)->_name = mpsList.at(i)._problem_mps;
      problems_and_data.emplace_back(xpansion_problems.at(i), mpsList.at(i));
    }
    std::for_each(std::execution::par, problems_and_data.begin(),
                  problems_and_data.end(), [&](const auto& problem_and_data) {
                    const auto& [problem, data] = problem_and_data;

                    auto problem_variables_from_zip_adapter =
                        std::make_shared<ProblemVariablesZipAdapter>(
                            reader, data, links, logger);
                    linkProblemsGenerator.treat(
                        data._problem_mps, couplings, problem_writer, problem,
                        problem_variables_from_zip_adapter);
                  });

    /* Clean up */
    reader->Close();
    reader->Delete();

    std::filesystem::remove(antares_archive_path);
    std::filesystem::rename(tmpArchivePath, lpDir_ / (MPS_ZIP_FILE + ZIP_EXT));
    writer->Close();
    writer->Delete();
  } else if (use_file_implementation) {
    /*Instantiate zip writer */
    auto lpDir_ = xpansion_output_dir / "lp";
    const auto tmpArchiveName = MPS_ZIP_FILE + "-tmp" + ZIP_EXT;
    const auto tmpArchivePath = lpDir_ / tmpArchiveName;
    std::filesystem::remove(tmpArchivePath);
    auto writer = std::make_shared<ArchiveWriter>(tmpArchivePath);
    writer->Open();

    /* Zip problem writer */
    auto problem_writer =
        std::make_shared<ArchiveProblemWriter>(xpansion_output_dir, writer);

    /* Main stuff */
    linkProblemsGenerator.treatloop_files(xpansion_output_dir, couplings,
                                          mpsList, problem_writer);

    std::filesystem::remove(antares_archive_path);
    std::filesystem::rename(tmpArchivePath, lpDir_ / (MPS_ZIP_FILE + ZIP_EXT));
    writer->Close();
    writer->Delete();
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
    auto xpansion_problems = adapter.provideProblems(solver_name);
    std::vector<std::pair<std::shared_ptr<Problem>, ProblemData>>
        problems_and_data;
    for (int i = 0; i < xpansion_problems.size(); ++i) {
      xpansion_problems.at(i)->_name = mpsList.at(i)._problem_mps;
      problems_and_data.emplace_back(xpansion_problems.at(i), mpsList.at(i));
    }

    auto reader = std::make_shared<ArchiveReader>(antares_archive_path);
    reader->Open();
    auto lpDir_ = xpansion_output_dir / "lp";
    const auto tmpArchiveName = MPS_ZIP_FILE + "-tmp" + ZIP_EXT;
    const auto tmpArchivePath = lpDir_ / tmpArchiveName;
    auto writer = std::make_shared<ArchiveWriter>(tmpArchivePath);
    writer->Open();
    auto problem_writer =
        std::make_shared<ArchiveProblemWriter>(lpDir_, writer);

    std::for_each(std::execution::par, problems_and_data.begin(),
                  problems_and_data.end(), [&](const auto& problem_and_data) {
                    const auto& [problem, data] = problem_and_data;

                    auto problem_variables_from_zip_adapter =
                        std::make_shared<ProblemVariablesZipAdapter>(
                            reader, data, links, logger);
                    linkProblemsGenerator.treat(
                        data._problem_mps, couplings, problem_writer, problem,
                        problem_variables_from_zip_adapter);
                  });
    /* Clean up */
    reader->Close();
    reader->Delete();

    std::filesystem::remove(antares_archive_path);
    std::filesystem::rename(tmpArchivePath, lpDir_ / (MPS_ZIP_FILE + ZIP_EXT));
    writer->Close();
    writer->Delete();
  }

  MasterGeneration master_generation(
      xpansion_output_dir, links, additionalConstraints, couplings,
      master_formulation, solver_name, logger, log_file_path);
}