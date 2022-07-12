//
// Created by marechaljas on 16/06/22.
//

#include "StudyUpdateStrategy.h"

static const std::filesystem::path ANTARES_LINK_FOLDER = std::filesystem::path("input") / "links";

void StudyUpdateStrategy::EnsureCandidateInvestmentFound(
    const std::map<std::string, double>& investments_p,
    const ActiveLink& link_p, const Candidate& candidate,
    const std::map<std::string, double>::const_iterator& it_candidate) const {
  if (it_candidate == investments_p.end()) {
    std::string message = "No investment computed for the candidate " +
        candidate.get_name() + " on the link " +
        link_p.get_LinkName();
    throw std::runtime_error(message);
  }
}
StudyUpdateStrategy::StudyUpdateStrategy(const std::filesystem::path& study_path)
: antares_link_folder_path{study_path / ANTARES_LINK_FOLDER}
{
}
