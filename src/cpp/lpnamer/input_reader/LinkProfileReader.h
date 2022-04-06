#ifndef ANTARESXPANSION_LINKPROFILEREADER_H
#define ANTARESXPANSION_LINKPROFILEREADER_H

#include <filesystem>

#include "Candidate.h"
#include "LinkProfile.h"
class LinkProfileReader {
 public:
  LinkProfileReader() = default;

  static LinkProfile ReadLinkProfile(const std::filesystem::path& filename);
  static const std::map<std::string, LinkProfile> getLinkProfileMap(
      const std::filesystem::path& capacity_folder,
      const std::vector<CandidateData>& candidateList);

 private:
  static void importProfile(std::map<std::string, LinkProfile>& mapLinkProfile,
                            const std::filesystem::path& capacitySubfolder,
                            const std::string& profile_name);
};

#endif  // ANTARESXPANSION_LINKPROFILEREADER_H
