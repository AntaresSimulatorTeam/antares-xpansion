#include "antares-xpansion/study-updater/StudyUpdater.h"

#include <utility>

#include "JsonXpansionReader.h"
#include "antares-xpansion/study-updater/StudyUpdateLinkCapacitiesStrategy.h"
#include "antares-xpansion/study-updater/StudyUpdateLinkParameterStrategy.h"
#include "antares-xpansion/study-updater/StudyUpdateStrategy.h"

constexpr int ANTARES_VERSION_CAPACITIES_IN_INDIVIDUAL_FILES = 820;

StudyUpdater::StudyUpdater(
    std::filesystem::path studyPath_p,
    const AntaresVersionProvider& antares_version_provider,
    ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger)
    : studyPath_(std::move(studyPath_p)), logger_(logger) {
  antaresVersion_ = antares_version_provider.getAntaresVersion(studyPath_);
}

int StudyUpdater::updateLinkdataFile(
    const ActiveLink& link_p,
    const std::map<std::string, double>& investments_p) const {
  if (antaresVersion_ >= ANTARES_VERSION_CAPACITIES_IN_INDIVIDUAL_FILES) {
    StudyUpdateLinkCapacitiesStrategy study_update(studyPath_, logger_);
    return study_update.Update(link_p, investments_p);
  } else {
    StudyUpdateLinkParameterStrategy study_update(studyPath_, logger_);
    return study_update.Update(link_p, investments_p);
  }
}

int StudyUpdater::update(std::vector<ActiveLink> const& links_p,
                         const std::filesystem::path& jsonPath_p) {
  JsonXpansionReader jsonReader_l;
  jsonReader_l.read(jsonPath_p);

  std::map<std::string, double> solution_l = jsonReader_l.getSolutionPoint();

  return update(links_p, solution_l);
}

int StudyUpdater::update(std::vector<ActiveLink> const& links_p,
                         const std::map<std::string, double>& investments_p) {
  int updateFailures_l(0);

  for (const auto& link : links_p) {
    updateFailures_l += updateLinkdataFile(link, investments_p);
  }

  return updateFailures_l;
}
