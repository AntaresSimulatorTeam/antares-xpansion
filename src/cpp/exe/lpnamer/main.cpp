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

#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>
#include <sstream>

#include "ActiveLinks.h"
#include "AdditionalConstraints.h"
#include "ArchiveProblemWriter.h"
#include "Candidate.h"
#include "CandidatesINIReader.h"
#include "LauncherHelpers.h"
#include "LinkProblemsGenerator.h"
#include "LinkProfileReader.h"
#include "MasterGeneration.h"
#include "MasterProblemBuilder.h"
#include "ProblemGenerationLogger.h"
#include "solver_utils.h"

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
    auto lpDir_ = root / "lp";
    auto reader = ArchiveReader(archivePath);
    reader.Open();
    const auto tmpArchiveName = MPS_ZIP_FILE + "-tmp" + ZIP_EXT;
    const auto tmpArchivePath = lpDir_ / tmpArchiveName;
    auto writer = std::make_shared<ArchiveWriter>(tmpArchivePath);
    writer->Open();

    LinkProblemsGenerator linkProblemsGenerator(links, solver_name, logger,
                                                log_file_path);
    auto mpsList = linkProblemsGenerator.readMPSList(mps_file_name);

    ArchiveProblemWriter problem_writer(root, writer);
    linkProblemsGenerator.treatloop(root, couplings, mpsList, &problem_writer,
                                    reader);

    std::filesystem::remove(archivePath);
    std::filesystem::rename(tmpArchivePath, lpDir_ / (MPS_ZIP_FILE + ZIP_EXT));
    reader.Close();
    reader.Delete();
    writer->Close();
    writer->Delete();


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
