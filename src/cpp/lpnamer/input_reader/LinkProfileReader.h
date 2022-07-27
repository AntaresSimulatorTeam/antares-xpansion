#ifndef ANTARESXPANSION_LINKPROFILEREADER_H
#define ANTARESXPANSION_LINKPROFILEREADER_H

#include <filesystem>

#include "Candidate.h"
#include "LinkProfile.h"
class LinkProfileReader {
 public:
  LinkProfileReader() = default;

  static std::vector<LinkProfile> ReadLinkProfile(const std::filesystem::path& direct_filename,
                                                       const std::filesystem::path& indirect_file_name);
  static std::vector<LinkProfile> ReadLinkProfile(const std::filesystem::path& direct_filename);
  static std::map<std::string, std::vector<LinkProfile>> getLinkProfileMap(
      const std::filesystem::path& capacity_folder,
      const std::vector<CandidateData>& candidateList);

 private:
  static void importProfile(
      std::map<std::string, std::vector<LinkProfile>>& mapLinkProfile,
      const std::filesystem::path& capacitySubfolder,
      const std::string& direct_profile_name,
      const std::string& indirect_profile_name);

  static void ReadLinkProfile(const std::filesystem::path & filename,
                                    std::vector<LinkProfile>& result,
                                    bool fillDirectProfile);

  static void ConstructChronicle(std::vector<LinkProfile>& result,
                                 int chronicle_id);
  static void UpdateProfile(std::vector<LinkProfile>& result,
                            bool directProfile, double value, int chronicle_id,
                            size_t time_step);
  static void EnsureFileIsGood(const std::filesystem::path& direct_filename);
};

#endif  // ANTARESXPANSION_LINKPROFILEREADER_H
