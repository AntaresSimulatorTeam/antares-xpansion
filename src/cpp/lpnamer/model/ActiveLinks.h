
#ifndef ANTARESXPANSION_ACTIVELINKS_H
#define ANTARESXPANSION_ACTIVELINKS_H

#include <Candidate.h>

#include <unordered_map>

#include "ChronicleMapProvider.h"
#include "LogUtils.h"
#include "ProblemGenerationLogger.h"

using LinkName = std::string;

class ActiveLink {
 public:
  ActiveLink(int idLink, const std::string& linkName, const std::string& linkor,
             const std::string& linkex,
             const double& already_installed_capacity,
             ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger);
  ActiveLink(int idLink, std::string linkName, std::string linkor,
             std::string linkex, const double& already_installed_capacity,
             std::map<unsigned, unsigned> mc_year_to_chronicle,
             ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger);

  void setAlreadyInstalledLinkProfiles(
      const std::vector<LinkProfile>& linkProfile);

  void addCandidate(const CandidateData& candidate_data,
                    const std::vector<LinkProfile>& candidate_profile);
  [[nodiscard]] const std::vector<Candidate>& getCandidates() const;

  [[nodiscard]] double already_installed_direct_profile(size_t timeStep) const;
  [[nodiscard]] double already_installed_indirect_profile(
      size_t timeStep) const;
  [[nodiscard]] double already_installed_direct_profile(size_t chronicle_number,
                                                        size_t timeStep) const;
  [[nodiscard]] double already_installed_indirect_profile(
      size_t chronicle_number, size_t timeStep) const;

  [[nodiscard]] unsigned get_idLink() const;
  [[nodiscard]] LinkName get_LinkName() const;
  [[nodiscard]] std::string get_linkor() const;
  [[nodiscard]] const std::string& linkor() const;
  [[nodiscard]] std::string get_linkex() const;
  [[nodiscard]] const std::string& linkex() const;
  [[nodiscard]] double get_already_installed_capacity() const;

  [[nodiscard]] std::map<unsigned int, unsigned int> McYearToChronicle() const {
    return mc_year_to_chronicle_;
  }

  [[nodiscard]] unsigned long number_of_chronicles() const;

 private:
  std::map<unsigned, unsigned> mc_year_to_chronicle_;
  unsigned int _idLink;
  LinkName _name;
  std::string _linkor;
  std::string _linkex;
  // Sur le lien capacité à ne pas toucher
  double _already_installed_capacity = 1;
  // Profile de la capa
  std::vector<LinkProfile> _already_installed_profile = {};
  std::vector<Candidate> _candidates = {};
  ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger_;
};

class ActiveLinksBuilder {
 public:
  ActiveLinksBuilder(
      std::vector<CandidateData> candidateList,
      std::map<std::string, std::vector<LinkProfile>> profile_map,
      DirectAccessScenarioToChronicleProvider scenario_to_chronicle_provider,
      ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger);

  ActiveLinksBuilder(
      const std::vector<CandidateData>& candidateList,
      const std::map<std::string, std::vector<LinkProfile>>& profile_map,
      ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger);

  class MultipleAlreadyInstalledCapacityDetectedForLink
      : public LogUtils::XpansionError<std::runtime_error> {
    using LogUtils::XpansionError<std::runtime_error>::XpansionError;
  };
  class MultipleAlreadyInstalledProfileDetectedForLink
      : public LogUtils::XpansionError<std::runtime_error> {
    using LogUtils::XpansionError<std::runtime_error>::XpansionError;
  };
  class MultipleLinkIddetectedForLink
      : public LogUtils::XpansionError<std::runtime_error> {
    using LogUtils::XpansionError<std::runtime_error>::XpansionError;
  };
  class CandidateDuplicationDetected
      : public LogUtils::XpansionError<std::runtime_error> {
    using LogUtils::XpansionError<std::runtime_error>::XpansionError;
  };
  class ThereIsNoLinkProfileAssociatedWithThisProfile
      : public LogUtils::XpansionError<std::runtime_error> {
    using LogUtils::XpansionError<std::runtime_error>::XpansionError;
  };

  const std::vector<ActiveLink>& getLinks();

 private:
  struct LinkData {
    int id;
    double installed_capacity;
    std::string profile_name;
    std::string _linkor;
    std::string _linkex;
  };

  void checkCandidateNameDuplication() const;
  void checkLinksValidity();

  int getLinkIndexOf(int link_id) const;
  void addCandidate(const CandidateData& candidate_data);
  void launchExceptionIfNoLinkProfileAssociated(
      const std::string& profileName) const;

  void record_link_data(const CandidateData& candidateData);
  void raise_errors_if_link_data_differs_from_existing_link(
      const LinkData& link_data, const LinkName& link_name) const;
  void create_links();

  LinkProfile getProfileFromProfileMap(const std::string& profile_name) const;
  std::vector<LinkProfile> getProfilesFromProfileMap(
      const std::string& profile_name) const;

  std::map<LinkName, LinkData> _links_data;
  std::unordered_map<LinkName, std::string> linkToAlreadyInstalledProfileName;
  std::unordered_map<LinkName, double> linkToAlreadyInstalledCapacity;
  const std::vector<CandidateData> _candidateDatas;
  const std::map<std::string, std::vector<LinkProfile>> _profile_map;
  std::vector<ActiveLink> _links;
  DirectAccessScenarioToChronicleProvider scenario_to_chronicle_provider_;
  ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger_;
};

#endif  // ANTARESXPANSION_ACTIVELINKS_H
