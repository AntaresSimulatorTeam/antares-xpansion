#include "antares-xpansion/lpnamer/input_reader/LinkProfileReader.h"

#include <filesystem>

#include "antares-xpansion/xpansion_interfaces/LogUtils.h"

std::vector<LinkProfile> LinkProfileReader::ReadLinkProfile(
    const std::filesystem::path &direct_filename,
    const std::filesystem::path &indirect_file_name) {
  (*logger_)(LogUtils::LOGLEVEL::DEBUG)
      << LOGLOCATION << "direct_filename : " << direct_filename << "\n"
      << "indirect_file_name: " << indirect_file_name << "\n";
  EnsureFileIsGood(direct_filename);
  EnsureFileIsGood(indirect_file_name);
  std::vector<LinkProfile> result;
  ReadLinkProfile(direct_filename, result, true);
  ReadLinkProfile(indirect_file_name, result, false);
  return result;
}
void LinkProfileReader::EnsureFileIsGood(
    const std::filesystem::path &direct_filename) const {
  if (std::ifstream infile(direct_filename); !infile.good()) {
    (*logger_)(LogUtils::LOGLEVEL::FATAL)
        << LOGLOCATION << "unable to open file" << direct_filename;
    throw std::filesystem::filesystem_error(LOGLOCATION + "unable to open file",
                                            direct_filename, std::error_code());
  }
}

std::vector<LinkProfile> LinkProfileReader::ReadLinkProfile(
    const std::filesystem::path &direct_filename) {
  std::vector<LinkProfile> result;
  ReadLinkProfile(direct_filename, result, true);
  ReadLinkProfile(direct_filename, result, false);
  return result;
}
bool is_number(std::string const &str, double &result) {
  auto i = std::istringstream(str);

  i >> result;

  return !i.fail() && i.eof();
}
void LinkProfileReader::ReadLinkProfile(const std::filesystem::path &filename,
                                        std::vector<LinkProfile> &result,
                                        bool fillDirectProfile) {
  std::ifstream infile(filename);
  if (!infile.good()) {
    auto errMsg = std::string("unable to open file ");
    (*logger_)(LogUtils::LOGLEVEL::FATAL)
        << LOGLOCATION << errMsg << filename;
    throw std::filesystem::filesystem_error(LOGLOCATION + errMsg, filename,
                                            std::error_code());
  }
  std::string str_value;
  double value;
  std::string line;
  int chronicle_id = 0;
  for (size_t time_step(0); time_step < NUMBER_OF_HOUR_PER_YEAR; ++time_step) {
    if (std::getline(infile, line)) {
      std::stringstream buffer(line);
      chronicle_id = 0;
      while (buffer >> str_value) {
        if (is_number(str_value, value)) {
          ConstructChronicle(result, chronicle_id);
          UpdateProfile(result, fillDirectProfile, value, chronicle_id,
                        time_step);
          ++chronicle_id;
        } else {
          auto errMsg =
              std::string("Error while reading value in link-profile ") +
              filename.string() + " line " + std::to_string(time_step) + "\n";
          (*logger_)(LogUtils::LOGLEVEL::FATAL) << LOGLOCATION << errMsg;
          throw std::domain_error(errMsg);
        }
      }
    } else {
      auto errMsg = std::string("error not enough line in link-profile ") +
                    filename.string();
      (*logger_)(LogUtils::LOGLEVEL::FATAL)
          << LOGLOCATION << errMsg;
      throw std::domain_error(errMsg);
    }
  }
  infile.close();
}
void LinkProfileReader::UpdateProfile(std::vector<LinkProfile> &result,
                                      bool directProfile, double value,
                                      int chronicle_id,
                                      size_t time_step) const {
  LinkProfile &profile = result.at(chronicle_id);
  if (directProfile) {
    profile.direct_link_profile.at(time_step) = value;
  } else {
    profile.indirect_link_profile.at(time_step) = value;
  }
}

void LinkProfileReader::ConstructChronicle(std::vector<LinkProfile> &result,
                                           int chronicle_id) const {
  if (result.size() <= chronicle_id) {
    result.emplace_back(logger_);
  }
}

std::map<std::string, std::vector<LinkProfile>>
LinkProfileReader::getLinkProfileMap(
    const std::filesystem::path &capacity_folder,
    const std::vector<CandidateData> &candidateList) {
  std::map<std::string, std::vector<LinkProfile>> mapLinkProfile;
  for (const auto &candidate_data : candidateList) {
    importProfile(mapLinkProfile, capacity_folder,
                  candidate_data.installed_direct_link_profile_name,
                  candidate_data.installed_indirect_link_profile_name);
    importProfile(mapLinkProfile, capacity_folder,
                  candidate_data.direct_link_profile,
                  candidate_data.indirect_link_profile);
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
    mapLinkProfile[direct_profile_name] = LinkProfileReader::ReadLinkProfile(
        capacitySubfolder / direct_profile_name,
        capacitySubfolder / indirect_profile_name);
  }
}