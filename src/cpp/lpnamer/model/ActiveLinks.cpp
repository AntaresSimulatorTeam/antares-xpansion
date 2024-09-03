#include "ActiveLinks.h"

#include <limits>
#include <unordered_set>
#include <utility>

bool doubles_are_different(const double a, const double b) {
  constexpr double MACHINE_EPSILON = std::numeric_limits<double>::epsilon();
  return std::abs(a - b) > MACHINE_EPSILON;
}

void ActiveLinksBuilder::addCandidate(const CandidateData& candidate_data) {
  unsigned int indexLink = getLinkIndexOf(candidate_data.link_id);
  _links.at(indexLink).addCandidate(
      candidate_data,
      getProfilesFromProfileMap(candidate_data.direct_link_profile));
}

ActiveLinksBuilder::ActiveLinksBuilder(
    std::vector<CandidateData> candidateList,
    std::map<std::string, std::vector<LinkProfile>> profile_map,
    DirectAccessScenarioToChronicleProvider scenario_to_chronicle_provider,
    ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger)
    : _candidateDatas(std::move(candidateList)),
      _profile_map(std::move(profile_map)),
      scenario_to_chronicle_provider_(
          std::move(scenario_to_chronicle_provider)),
      logger_(logger) {
  checkCandidateNameDuplication();
  checkLinksValidity();
}

ActiveLinksBuilder::ActiveLinksBuilder(
    const std::vector<CandidateData>& candidateList,
    const std::map<std::string, std::vector<LinkProfile>>& profile_map,
    ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger)
    : ActiveLinksBuilder(candidateList, profile_map,
                         DirectAccessScenarioToChronicleProvider("", logger),
                         logger) {}

void ActiveLinksBuilder::checkLinksValidity() {
  for (const auto& candidateData : _candidateDatas) {
    launchExceptionIfNoLinkProfileAssociated(candidateData.direct_link_profile);
    launchExceptionIfNoLinkProfileAssociated(
        candidateData.installed_direct_link_profile_name);

    record_link_data(candidateData);
  }
}

void ActiveLinksBuilder::record_link_data(const CandidateData& candidateData) {
  LinkData link_data = {candidateData.link_id,
                        candidateData.already_installed_capacity,
                        candidateData.installed_direct_link_profile_name,
                        candidateData.linkor, candidateData.linkex};
  const auto& it = _links_data.find(candidateData.link_name);
  if (it == _links_data.end()) {
    _links_data[candidateData.link_name] = link_data;
  } else {
    const LinkName link_name = it->first;
    raise_errors_if_link_data_differs_from_existing_link(link_data, link_name);
  }
}

void ActiveLinksBuilder::raise_errors_if_link_data_differs_from_existing_link(
    const ActiveLinksBuilder::LinkData& link_data,
    const LinkName& link_name) const {
  const LinkData& old_link_data = _links_data.at(link_name);
  if (doubles_are_different(link_data.installed_capacity,
                            old_link_data.installed_capacity)) {
    auto log_location = LOGLOCATION;
    auto message =
        "Multiple already installed capacity detected for link " + link_name;
    (*logger_)(LogUtils::LOGLEVEL::FATAL)
        << log_location << message;
    throw MultipleAlreadyInstalledCapacityDetectedForLink(message,
                                                          log_location);
  }
  if (old_link_data.profile_name != link_data.profile_name) {
    auto log_location = LOGLOCATION;
    auto message =
        "Multiple already_installed_profile detected for link " + link_name;
    (*logger_)(LogUtils::LOGLEVEL::FATAL)
        << log_location << message;
    throw MultipleAlreadyInstalledProfileDetectedForLink(message, log_location);
  }
  if (old_link_data.id != link_data.id) {
    auto log_location = LOGLOCATION;
    auto message = "Multiple link_id detected for link " + link_name;
    (*logger_)(LogUtils::LOGLEVEL::FATAL)
        << log_location << message;
    throw MultipleLinkIddetectedForLink(message, log_location);
  }
}

void ActiveLinksBuilder::launchExceptionIfNoLinkProfileAssociated(
    const std::string& profileName) const {
  if (!profileName.empty()) {
    const auto& it_profile = _profile_map.find(profileName);

    if (it_profile == _profile_map.end()) {
      auto log_location = LOGLOCATION;
      auto message = "There is no linkProfile associated with " + profileName;
      (*logger_)(LogUtils::LOGLEVEL::FATAL)
          << log_location << message;
      throw ThereIsNoLinkProfileAssociatedWithThisProfile(message,
                                                          log_location);
    }
  }
}

void ActiveLinksBuilder::checkCandidateNameDuplication() const {
  std::unordered_set<std::string> setCandidatesNames;
  for (const auto& candidateData : _candidateDatas) {
    auto it_inserted = setCandidatesNames.insert(candidateData.name);
    if (!it_inserted.second) {
      auto log_location = LOGLOCATION;
      auto message =
          "Candidate " + candidateData.name + " duplication detected";
      (*logger_)(LogUtils::LOGLEVEL::FATAL)
          << log_location << message;
      throw CandidateDuplicationDetected(message, log_location);
    }
  }
}

const std::vector<ActiveLink>& ActiveLinksBuilder::getLinks() {
  if (_links.empty()) {
    create_links();
    for (const CandidateData& candidateData : _candidateDatas) {
      if (candidateData.enable) {
        addCandidate(candidateData);
      }
    }
  }
  return _links;
}

int ActiveLinksBuilder::getLinkIndexOf(int link_id) const {
  int index = -1;
  for (int i = 0; i < _links.size(); i++) {
    if (_links.at(i).get_idLink() == link_id) {
      index = i;
      break;
    }
  }
  return index;
}

void ActiveLinksBuilder::create_links() {
  for (auto const& [link_name, link_data] : _links_data) {
    auto mc_year_to_chronicle = scenario_to_chronicle_provider_.GetMap(
        link_data._linkor, link_data._linkex);
    ActiveLink link(link_data.id, link_name, link_data._linkor,
                    link_data._linkex, link_data.installed_capacity,
                    mc_year_to_chronicle, logger_);
    link.setAlreadyInstalledLinkProfiles(
        getProfilesFromProfileMap(link_data.profile_name));
    _links.push_back(link);
  }
}

std::vector<LinkProfile> ActiveLinksBuilder::getProfilesFromProfileMap(
    const std::string& profile_name) const {
  std::vector<LinkProfile> profiles;
  if (_profile_map.find(profile_name) != _profile_map.end()) {
    profiles = _profile_map.at(profile_name);
  }
  if (profiles.empty()) return {LinkProfile(logger_)};
  return profiles;
}

ActiveLink::ActiveLink(
    int idLink, std::string linkName, std::string linkor, std::string linkex,
    const double& already_installed_capacity,
    std::map<unsigned int, unsigned int> mc_year_to_chronicle,
    ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger)
    : mc_year_to_chronicle_(std::move(mc_year_to_chronicle)),
      _idLink(idLink),
      _name(std::move(linkName)),
      _linkor(std::move(linkor)),
      _linkex(std::move(linkex)),
      _already_installed_capacity(already_installed_capacity),
      logger_(logger) {
  _already_installed_profile.emplace_back(logger_);
}

ActiveLink::ActiveLink(
    int idLink, const std::string& linkName, const std::string& linkor,
    const std::string& linkex, const double& already_installed_capacity,
    ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger)
    : ActiveLink(idLink, linkName, linkor, linkex, already_installed_capacity,
                 {}, logger) {}

void ActiveLink::setAlreadyInstalledLinkProfiles(
    const std::vector<LinkProfile>& linkProfile) {
  _already_installed_profile = linkProfile;
}

void ActiveLink::addCandidate(
    const CandidateData& candidate_data,
    const std::vector<LinkProfile>& candidate_profile) {
  Candidate candidate(candidate_data, candidate_profile);

  _candidates.push_back(candidate);
}

const std::vector<Candidate>& ActiveLink::getCandidates() const {
  return _candidates;
}

double ActiveLink::already_installed_direct_profile(size_t timeStep) const {
  return _already_installed_profile.at(0).getDirectProfile(timeStep);
}

double ActiveLink::already_installed_indirect_profile(size_t timeStep) const {
  return _already_installed_profile.at(0).getIndirectProfile(timeStep);
}

double ActiveLink::already_installed_direct_profile(size_t chronicle_number,
                                                    size_t timeStep) const {
  if (chronicle_number == 0) chronicle_number = 1;
  if (chronicle_number > _already_installed_profile.size())
    chronicle_number = 1;
  return _already_installed_profile.at(chronicle_number - 1)
      .getDirectProfile(timeStep);
}

double ActiveLink::already_installed_indirect_profile(size_t chronicle_number,
                                                      size_t timeStep) const {
  if (chronicle_number == 0) chronicle_number = 1;
  if (chronicle_number > _already_installed_profile.size())
    chronicle_number = 1;
  return _already_installed_profile.at(chronicle_number - 1)
      .getIndirectProfile(timeStep);
}

unsigned ActiveLink::get_idLink() const { return _idLink; }

LinkName ActiveLink::get_LinkName() const { return _name; }

double ActiveLink::get_already_installed_capacity() const {
  return _already_installed_capacity;
}

std::string ActiveLink::get_linkor() const { return _linkor; }
const std::string& ActiveLink::linkor() const { return _linkor; }

std::string ActiveLink::get_linkex() const { return _linkex; }
const std::string& ActiveLink::linkex() const { return _linkex; }

unsigned long ActiveLink::number_of_chronicles() const {
  // We don't check for correctness of the number of chronicles across profiles
  // We assume that all profiles have either 1 chronicle (per default) or the
  // same N number of chronicles. We can have 1 installed chronicle and N
  // profile chronicle or vice versa
  if (unsigned long number_of_chronicles = _already_installed_profile.size();
      number_of_chronicles > 1)
    return number_of_chronicles;
  if (auto candidates = getCandidates(); !candidates.empty()) {
    return candidates.at(0).number_of_chronicles();  // Either 1 or N
  }
  return 1;
}
