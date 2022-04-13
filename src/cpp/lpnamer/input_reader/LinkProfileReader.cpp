#include "LinkProfileReader.h"

#include <filesystem>

std::vector<LinkProfile> LinkProfileReader::ReadLinkProfile(
    const std::filesystem::path &direct_filename, const std::filesystem::path& indirect_file_name) {
  if (std::ifstream infile(direct_filename); !infile.good()) {
    throw std::filesystem::filesystem_error("unable to open file",
                                            direct_filename, std::error_code());
  }
  std::vector<LinkProfile> result;
  ReadLinkProfile(direct_filename, result, true);
  ReadLinkProfile(indirect_file_name, result, false);
  return result;
}

std::vector<LinkProfile> LinkProfileReader::ReadLinkProfile(
    const std::filesystem::path &direct_filename) {
  std::vector<LinkProfile> result;
  ReadLinkProfile(direct_filename, result, true);
  ReadLinkProfile(direct_filename, result, false);
  return result;
}

void LinkProfileReader::ReadLinkProfile(const std::filesystem::path &filename,
                                              std::vector<LinkProfile> &result,
                                              bool fillDirectProfile) {
  std::ifstream infile(filename);
  if (!infile.good()) {
    throw std::filesystem::filesystem_error("unable to open file", filename, std::error_code());
  }
  double value;
  std::string line;
  int chronicle_id = 0;
  for (size_t line_id(0); line_id < NUMBER_OF_HOUR_PER_YEAR; ++line_id) {
    if (std::getline(infile, line)) {
      std::stringstream buffer(line);
      chronicle_id = 0;
      while ( buffer >> value ) {
        UpdateResultWithMissingChronicle(result, chronicle_id);
        UpdateProfile(result, fillDirectProfile, value, chronicle_id, line_id);
        ++chronicle_id;
      }
    } else {
      throw std::domain_error("error not enough line in link-profile " +
                              filename.string());
    }
  }
  infile.close();
}
void LinkProfileReader::UpdateProfile(std::vector<LinkProfile> &result,
                                      bool directProfile, double value,
                                      int chronicle_id, size_t line_id) {
  LinkProfile &profile = result.at(chronicle_id);
  if (directProfile) {
    profile.direct_link_profile.at(line_id) = value;
  } else {
    profile.indirect_link_profile.at(line_id) = value;
  }
}

void LinkProfileReader::UpdateResultWithMissingChronicle(
    std::vector<LinkProfile> &result, int chronicle_id) {
  if (result.size() <= chronicle_id) {
    result.emplace_back();
  }
}

std::map<std::string, std::vector<LinkProfile>> LinkProfileReader::getLinkProfileMap(
    const std::filesystem::path &capacity_folder,
    const std::vector<CandidateData> &candidateList) {
  std::map<std::string, std::vector<LinkProfile>> mapLinkProfile;
  for (const auto &candidate_data : candidateList) {
    importProfile(mapLinkProfile, capacity_folder,
                  candidate_data.installed_direct_link_profile_name, candidate_data.installed_indirect_link_profile_name);
    importProfile(mapLinkProfile, capacity_folder,
                  candidate_data.direct_link_profile, candidate_data.indirect_link_profile);
  }
  return mapLinkProfile;
}

void LinkProfileReader::importProfile(
    std::map<std::string, std::vector<LinkProfile>> &mapLinkProfile,
    const std::filesystem::path &capacitySubfolder,
    const std::string &direct_profile_name,
    const std::string &indirect_profile_name) {
  if (!direct_profile_name.empty() &&
      mapLinkProfile.find(direct_profile_name) == mapLinkProfile.end()) {
    mapLinkProfile[direct_profile_name] =
        LinkProfileReader::ReadLinkProfile(capacitySubfolder / direct_profile_name, capacitySubfolder / indirect_profile_name);
  }
}