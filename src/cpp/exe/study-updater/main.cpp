
#include <boost/program_options.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

#include "ActiveLinks.h"
#include "antares-xpansion/lpnamer/input_reader/CandidatesINIReader.h"
#include "LauncherHelpers.h"
#include "antares-xpansion/lpnamer/input_reader/LinkProfileReader.h"
#include "antares-xpansion/lpnamer/helper/ProblemGenerationLogger.h"
#include "StudyUpdateRunner.h"
#include "StudyUpdater.h"

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
    std::filesystem::path xpansion_output_dir;
    std::filesystem::path solutionFile_l;
    auto study_updater_options_parser = StudyUpdaterExeOptions();
    study_updater_options_parser.Parse(argc, argv);

    xpansion_output_dir = study_updater_options_parser.XpansionOutputDir();
    solutionFile_l = study_updater_options_parser.SolutionFile();
    using namespace ProblemGenerationLog;
    auto log_file_path = xpansion_output_dir / "lp" / "StudyUpdateLog.txt";
    auto logger = ProblemGenerationLog::BuildLogger(log_file_path, std::cout,
                                                    "Study Update");
    ActiveLinksBuilder linksBuilder =
        get_link_builders(xpansion_output_dir, logger);

    const std::vector<ActiveLink> links = linksBuilder.getLinks();

    updateStudy(xpansion_output_dir, links, solutionFile_l, logger);

    return 0;
  } catch (std::exception &e) {
    std::cerr << "error: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Exception of unknown type!" << std::endl;
  }

  return 0;
}
