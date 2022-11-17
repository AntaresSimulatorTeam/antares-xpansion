/**
 * \file lpnamer/main.cpp
 * \brief POC Antares Xpansion V2
 * \author {Manuel Ruiz; Luc Di Gallo}
 * \version 0.1
 * \date 07 aout 2019
 *
 * POC Antares Xpansion V2: create inputs for the solver version 2
 *
 */

#include <algorithm>
#include <boost/archive/text_iarchive.hpp>
#include <boost/program_options.hpp>
#include <execution>
#include <iostream>

#include "ActiveLinks.h"
#include "AdditionalConstraints.h"
#include "ArchiveProblemWriter.h"
#include "LauncherHelpers.h"
#include "LinkProblemsGenerator.h"
#include "LpsFromAntares.h"
#include "MasterGeneration.h"
#include "MasterProblemBuilder.h"
#include "ProblemGenerationLogger.h"
#include "ProblemVariablesZipAdapter.h"
#include "XpansionProblemsFromAntaresProvider.h"
#include "ZipProblemsProviderAdapter.h"

namespace po = boost::program_options;

/**
 * \fn int main (void)
 * \brief Main program
 *
 * \param  argc An integer argument count of the command line arguments
 * \param  argv Path to input data which is the 1st argument vector of the
 * command line argument. \return an integer 0 corresponding to exit success
 */
int main(int argc, char **argv) {
  try {
    std::filesystem::path root;
    std::filesystem::path archivePath;
    std::string master_formulation;
    std::string additionalConstraintFilename_l;

    po::options_description desc("Allowed options");

    desc.add_options()("help,h", "produce help message")(
        "output,o", po::value<std::filesystem::path>(&root)->required(),
        "antares-xpansion study output")(
        "archive,a", po::value<std::filesystem::path>(&archivePath)->required(),
        "antares-xpansion study zip")(
        "formulation,f",
        po::value<std::string>(&master_formulation)->default_value("relaxed"),
        "master formulation (relaxed or integer)")(
        "exclusion-files,e",
        po::value<std::string>(&additionalConstraintFilename_l),
        "path to exclusion files");

    po::variables_map opts;
    po::store(po::parse_command_line(argc, argv, desc), opts);

    if (opts.count("help")) {
      std::cout << desc << std::endl;
      return 0;
    }

    po::notify(opts);
    /**/
    using namespace ProblemGenerationLog;
    const auto log_file_path = root / "lp" / "ProblemGenerationLog.txt";
    auto logFile = std::make_shared<ProblemGenerationFileLogger>(log_file_path);

    auto logStd = std::make_shared<ProblemGenerationOstreamLogger>(std::cout);

    auto logger = std::make_shared<ProblemGenerationLogger>(LOGLEVEL::INFO);
    logger->AddLogger(logFile);
    logger->AddLogger(logStd);
    auto &loggerRef = (*logger);

    /**/
    ActiveLinksBuilder linkBuilder = get_link_builders(root, logger);

    if ((master_formulation != "relaxed") &&
        (master_formulation != "integer")) {
      loggerRef(LOGLEVEL::FATAL)
          << "Invalid formulation argument : argument must be "
             "\"integer\" or \"relaxed\""
          << std::endl;
      std::exit(1);
    }

    AdditionalConstraints additionalConstraints(logger);
    if (!additionalConstraintFilename_l.empty()) {
      additionalConstraints.SetConstraintsFile(additionalConstraintFilename_l);
      additionalConstraints.ReadConstraintsFile();
    }

    Couplings couplings;
    std::string solver_name = "CBC";
    std::vector<ActiveLink> links = linkBuilder.getLinks();

    auto const mps_file_name = root / MPS_TXT;

    LinkProblemsGenerator linkProblemsGenerator(links, solver_name, logger,
                                                log_file_path);
    auto mpsList = linkProblemsGenerator.readMPSList(mps_file_name);

    bool use_zip_implementation = true;
    bool use_file_implementation = false;
    if (use_zip_implementation) {
      /* Instantiate Zip reader */
      auto reader = std::make_shared<ArchiveReader>(archivePath);
      reader->Open();

      /*Instantiate zip writer */
      auto lpDir_ = root / "lp";
      const auto tmpArchiveName = MPS_ZIP_FILE + "-tmp" + ZIP_EXT;
      const auto tmpArchivePath = lpDir_ / tmpArchiveName;
      auto writer = std::make_shared<ArchiveWriter>(tmpArchivePath);
      writer->Open();

      /* Zip problem writer */
      auto problem_writer =
          std::make_shared<ArchiveProblemWriter>(root, writer);

      /* Main stuff */
      std::vector<std::string> problem_names;
      std::transform(mpsList.begin(), mpsList.end(),
                     std::back_inserter(problem_names),
                     [](ProblemData const &data) { return data._problem_mps; });
      auto adapter = std::make_shared<ZipProblemsProviderAdapter>(
          lpDir_, reader, problem_names);
      std::vector<std::shared_ptr<Problem>> xpansion_problems =
          adapter->provideProblems(solver_name);

      std::vector<std::pair<std::shared_ptr<Problem>, ProblemData>>
          problems_and_data;
      for (int i = 0; i < xpansion_problems.size(); ++i) {
        xpansion_problems.at(i)->_name = mpsList.at(i)._problem_mps;
        problems_and_data.emplace_back(xpansion_problems.at(i), mpsList.at(i));
      }
      std::for_each(std::execution::par, problems_and_data.begin(),
                    problems_and_data.end(), [&](const auto &problem_and_data) {
                      const auto &[problem, data] = problem_and_data;

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

      std::filesystem::remove(archivePath);
      std::filesystem::rename(tmpArchivePath,
                              lpDir_ / (MPS_ZIP_FILE + ZIP_EXT));
      writer->Close();
      writer->Delete();
    } else if (use_file_implementation) {
      /*Instantiate zip writer */
      auto lpDir_ = root / "lp";
      const auto tmpArchiveName = MPS_ZIP_FILE + "-tmp" + ZIP_EXT;
      const auto tmpArchivePath = lpDir_ / tmpArchiveName;
      std::filesystem::remove(tmpArchivePath);
      auto writer = std::make_shared<ArchiveWriter>(tmpArchivePath);
      writer->Open();

      /* Zip problem writer */
      auto problem_writer =
          std::make_shared<ArchiveProblemWriter>(root, writer);

      /* Main stuff */
      linkProblemsGenerator.treatloop_files(root, couplings, mpsList,
                                            problem_writer);

      std::filesystem::remove(archivePath);
      std::filesystem::rename(tmpArchivePath,
                              lpDir_ / (MPS_ZIP_FILE + ZIP_EXT));
      writer->Close();
      writer->Delete();
    } else {
      std::string path =
          root.parent_path().parent_path() / "fichierDeSerialisation";
      std::ifstream ifs(root.parent_path().parent_path() /
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

      auto reader = std::make_shared<ArchiveReader>(archivePath);
      reader->Open();
      auto lpDir_ = root / "lp";
      const auto tmpArchiveName = MPS_ZIP_FILE + "-tmp" + ZIP_EXT;
      const auto tmpArchivePath = lpDir_ / tmpArchiveName;
      auto writer = std::make_shared<ArchiveWriter>(tmpArchivePath);
      writer->Open();
      auto problem_writer =
          std::make_shared<ArchiveProblemWriter>(lpDir_, writer);

      std::for_each(std::execution::par, problems_and_data.begin(),
                    problems_and_data.end(), [&](const auto &problem_and_data) {
                      const auto &[problem, data] = problem_and_data;

                      auto problem_variables_from_zip_adapter =
                          std::make_shared<ProblemVariablesZipAdapter>(
                              reader, data, links, logger);
                      linkProblemsGenerator.treat(
                          data._problem_mps, couplings, problem_writer, problem,
                          problem_variables_from_zip_adapter);
                    });

      std::cout << "plop";
      /* Clean up */
      reader->Close();
      reader->Delete();

      std::filesystem::remove(archivePath);
      std::filesystem::rename(tmpArchivePath,
                              lpDir_ / (MPS_ZIP_FILE + ZIP_EXT));
      writer->Close();
      writer->Delete();
    }


    MasterGeneration master_generation(root, links, additionalConstraints,
                                       couplings, master_formulation,
                                       solver_name, logger, log_file_path);

    return 0;
  } catch (std::exception &e) {
    std::cerr << "error: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Exception of unknown type!" << std::endl;
  }

  return 0;
}
