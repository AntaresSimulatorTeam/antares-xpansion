#pragma once

#include <filesystem>

#include "ActiveLinks.h"
#include "AntaresVersionProvider.h"
#include "LinkProblemsGenerator.h"
#include "ProblemGenerationLogger.h"

/*!
 * \class StudyUpdater
 * \brief Class that updates an antares study after an antares-xpansion
 * execution
 */
class StudyUpdater {
 private:
  // path to the antares study
  std::filesystem::path studyPath_;
  // antares version
  int antaresVersion_;
  ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger_;
  ProblemGenerationLog::ProblemGenerationLogger& loggerRef_ = *logger_;

 public:
  /*!
   * \brief constructor of class StudyUpdater
   *
   * \param studyPath_p : path to the antares study folder
   */
  explicit StudyUpdater(
      std::filesystem::path studyPath_p,
      const AntaresVersionProvider& antares_version_provider,
      ProblemGenerationLog::ProblemGenerationLoggerSharedPointer& logger);

  void setStudyPath(const std::filesystem::path& studyPath_p) {
    studyPath_ = std::move(studyPath_p);
  }

  void setAntaresVersionProvider(
      const AntaresVersionProvider& antares_version_provider) {
    antaresVersion_ = antares_version_provider.getAntaresVersion(studyPath_);
  }
  void setlogger(
      ProblemGenerationLog::ProblemGenerationLoggerSharedPointer& logger) {
    logger_ = logger;
  }
  /*!
   * \brief default destructor of calass StudyUpdater
   */
  virtual ~StudyUpdater() = default;

  /*!
   * \brief updates the linkdata file for a given link according to the
   * investments of its candidates
   *
   * \param link_p : link for which the linkdata file will be updated
   * \param investment_p : value of investment to consider for the update
   *
   * \return 1 if the update fails, 0 otherwise
   */

  [[nodiscard]] int updateLinkdataFile(
      const ActiveLink& link_p,
      const std::map<std::string, double>& investments_p);

  /*!
   * \brief updates the linkdata files for multiple candidates from a solution
   * Point
   *
   * \param candidates_p : the candidates for which the linkdata file will be
   * updated \param investments_p : a map giving investments per candidate
   *
   * \return number of candidates we failed to update
   */
  [[nodiscard]] int update(std::vector<ActiveLink> const& links_p,
                           const std::map<std::string, double>& investments_p);

  /*!
   * \brief updates the linkdata files for multiple candidates from a json file
   *
   * \param candidates_p : the candidates for which the linkdata file will be
   * updated \param jsonPath_p : path to a json file to read the investment
   * solution from
   *
   * \return number of candidates we failed to update
   */
  int update(std::vector<ActiveLink> const& links_p,
             std::string const& jsonPath_p);
};
