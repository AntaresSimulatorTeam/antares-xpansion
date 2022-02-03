#include "LinkProfileReader.h"

#include "helpers/Path.h"
LinkProfile LinkProfileReader::ReadLinkProfile(const std::string &filename) {
  std::ifstream infile(filename);
  if (!infile.good()) {
    throw std::runtime_error("unable to open : " + filename);
  }
  LinkProfile result;
  result._directLinkProfile.reserve(8760);
  double value;
  std::string line;
  for (size_t line_id(0); line_id < 8760; ++line_id) {
    if (std::getline(infile, line)) {
      std::stringstream buffer(line);
      buffer >> value;
      result._directLinkProfile.push_back(value);
      if (buffer >> value) {
        result._indirectLinkProfile.reserve(8760);
        result._indirectLinkProfile.push_back(value);
      }
    } else {
      throw std::runtime_error("error not enough line in link-profile " +
                               filename);
    }
  }
  infile.close();

  return result;
}

const std::map<std::string, LinkProfile> LinkProfileReader::getLinkProfileMap(
    const std::string &capacity_folder,
    const std::vector<CandidateData> &candidateList) {
  std::map<std::string, LinkProfile> mapLinkProfile;
  for (const auto &candidate_data : candidateList) {
    importProfile(mapLinkProfile, capacity_folder,
                  candidate_data.installed_link_profile_name);
    importProfile(mapLinkProfile, capacity_folder, candidate_data.link_profile);
  }
  return mapLinkProfile;
}

void LinkProfileReader::importProfile(
    std::map<std::string, LinkProfile> &mapLinkProfile,
    const std::string &capacitySubfolder, const std::string &profile_name) {
  if (!profile_name.empty() &&
      mapLinkProfile.find(profile_name) == mapLinkProfile.end()) {
    mapLinkProfile[profile_name] = LinkProfileReader::ReadLinkProfile(
        (Path(capacitySubfolder) / profile_name).get_str());
  }
}
