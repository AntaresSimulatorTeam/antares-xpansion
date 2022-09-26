
#include <boost/program_options.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

#include "ActiveLinks.h"
#include "CandidatesINIReader.h"
#include "LauncherHelpers.h"
#include "LinkProfileReader.h"
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
    std::filesystem::path root;
    std::filesystem::path solutionFile_l;
    auto study_updater_options_parser = StudyUpdaterExeOptions();
    study_updater_options_parser.Parse(argc, argv);

    root = study_updater_options_parser.Root();
    solutionFile_l = study_updater_options_parser.SolutionFile();
    ActiveLinksBuilder linksBuilder = get_link_builders(root);

    const std::vector<ActiveLink> links = linksBuilder.getLinks();

    updateStudy(root, links, solutionFile_l);

    return 0;
  } catch (std::exception &e) {
    std::cerr << "error: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Exception of unknown type!" << std::endl;
  }

  return 0;
}
