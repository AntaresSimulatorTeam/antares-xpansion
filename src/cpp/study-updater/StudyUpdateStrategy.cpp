//
// Created by marechaljas on 16/06/22.
//

#include "antares-xpansion/study-updater/StudyUpdateStrategy.h"

static const std::filesystem::path ANTARES_LINK_FOLDER =
    std::filesystem::path("input") / "links";

void StudyUpdateStrategy::EnsureCandidateInvestmentFound(
    const std::map<std::string, double>& investments_p,
    const ActiveLink& link_p, const Candidate& candidate,
    const std::map<std::string, double>::const_iterator& it_candidate) const {
  if (it_candidate == investments_p.end()) {
    auto log_location = LOGLOCATION;
    auto message = "No investment computed for the candidate " +
                   candidate.get_name() + " on the link " +
                   link_p.get_LinkName();
    (*logger_)(LogUtils::LOGLEVEL::FATAL)
        << log_location << message;
    throw NoInvestmentComputedForTheCandidate(message, log_location);
  }
}
StudyUpdateStrategy::StudyUpdateStrategy(
    const std::filesystem::path& study_path,
    ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger)
    : antares_link_folder_path{study_path / ANTARES_LINK_FOLDER},
      logger_(logger) {}
