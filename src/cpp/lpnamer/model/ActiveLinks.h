
#ifndef ANTARESXPANSION_ACTIVELINKS_H
#define ANTARESXPANSION_ACTIVELINKS_H

#include <Candidate.h>

#include <unordered_map>

using LinkName = std::string;

class ActiveLink {
 public:
  ActiveLink(int idLink, const std::string& linkName, const std::string& linkor,
             const std::string& linkex,
             const double& already_installed_capacity);
  void setAlreadyInstalledLinkProfile(const LinkProfile& linkProfile);

  void addCandidate(const CandidateData& candidate_data,
                    const std::vector<LinkProfile>& candidate_profile);
  const std::vector<Candidate>& getCandidates() const;

  double already_installed_direct_profile(size_t timeStep) const;
  double already_installed_indirect_profile(size_t timeStep) const;
  [[nodiscard]] double already_installed_direct_profile(size_t year, size_t timeStep) const;
  [[nodiscard]] double already_installed_indirect_profile(size_t year, size_t timeStep) const;

  int get_idLink() const;
  LinkName get_LinkName() const;
  std::string get_linkor() const;
  std::string get_linkex() const;
  double get_already_installed_capacity() const;

  std::map<unsigned, unsigned > mc_year_to_chronicle;

 private:
  int _idLink;
  LinkName _name;
  std::string _linkor;
  std::string _linkex;
  //Sur le lien capacité à ne pas toucher
  double _already_installed_capacity = 1;
  //Profile de la capa
  std::vector<LinkProfile> _already_installed_profile = {};
  std::vector<Candidate> _candidates = {};
};

class ActiveLinksBuilder {
 public:
  ActiveLinksBuilder(const std::vector<CandidateData>& candidateList,
      const std::map<std::string, std::vector<LinkProfile>>& profile_map);

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
  std::vector<LinkProfile> getProfilesFromProfileMap(const std::string& profile_name) const;

  std::map<LinkName, LinkData> _links_data;
  std::unordered_map<LinkName, std::string> linkToAlreadyInstalledProfileName;
  std::unordered_map<LinkName, double> linkToAlreadyInstalledCapacity;
  const std::vector<CandidateData> _candidateDatas;
  const std::map<std::string, std::vector<LinkProfile>> _profile_map;
  std::vector<ActiveLink> _links;
};

#endif  // ANTARESXPANSION_ACTIVELINKS_H
