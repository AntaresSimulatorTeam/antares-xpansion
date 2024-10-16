#include "antares-xpansion/study-updater/StudyUpdateRunner.h"

namespace po = boost::program_options;
StudyUpdaterExeOptions::StudyUpdaterExeOptions()
    : OptionsParser("Study-updater exe") {
  AddOptions()("help,h", "produce help message")(
      "study-output,o", po::value<std::filesystem::path>(&xpansion_output_dir_)->required(),
      "antares-xpansion study output")(
      "solution,s",
      po::value<std::filesystem::path>(&solution_file_)->required(),
      "path to json solution file");
}

/*!
 * \brief update links in the antares study directory
 *
 * \param rootPath_p path corresponding to the path to the simulation output
 * directory containing the lp directory \param links_p Structure which contains
 * the list of links \param jsonPath_l path to the json output file \return void
 */
void updateStudy(
    const std::filesystem::path &rootPath_p,
    const std::vector<ActiveLink> &links_p,
    const std::filesystem::path &jsonPath_l,
    ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger) {
  auto linksPath_l = rootPath_p / ".." / "..";

  StudyUpdater studyUpdater(linksPath_l, AntaresVersionProvider(), logger);
  int updateFailures_l = studyUpdater.update(links_p, jsonPath_l);

  if (updateFailures_l) {
    (*logger)(LogUtils::LOGLEVEL::ERR)
        << "Failed to update " << updateFailures_l << " files."
        << links_p.size() - updateFailures_l << " files were updated\n";
  }
}