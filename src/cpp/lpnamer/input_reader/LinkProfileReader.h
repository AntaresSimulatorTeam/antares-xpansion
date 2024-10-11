#ifndef ANTARESXPANSION_LINKPROFILEREADER_H
#define ANTARESXPANSION_LINKPROFILEREADER_H

#include <filesystem>
#include <map>
#include <utility>

#include "Candidate.h"
#include "LinkProfile.h"
#include "antares-xpansion/lpnamer/helper/ProblemGenerationLogger.h"

class LinkProfileReader {
 public:
  explicit LinkProfileReader(
      ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger)
      : logger_(std::move(logger)) {}

  std::vector<LinkProfile> ReadLinkProfile(
      const std::filesystem::path& direct_filename,
      const std::filesystem::path& indirect_file_name);
  std::vector<LinkProfile> ReadLinkProfile(
      const std::filesystem::path& direct_filename);
  std::map<std::string, std::vector<LinkProfile>> getLinkProfileMap(
      const std::filesystem::path& capacity_folder,
      const std::vector<CandidateData>& candidateList);

 private:
  void importProfile(
      std::map<std::string, std::vector<LinkProfile>>& mapLinkProfile,
      const std::filesystem::path& capacitySubfolder,
      const std::string& direct_profile_name,
      const std::string& indirect_profile_name);

  void ReadLinkProfile(const std::filesystem::path& filename,
                       std::vector<LinkProfile>& result,
                       bool fillDirectProfile);

  void ConstructChronicle(std::vector<LinkProfile>& result, int chronicle_id)const;
  void UpdateProfile(std::vector<LinkProfile>& result, bool directProfile,
                     double value, int chronicle_id, size_t time_step)const;
  void EnsureFileIsGood(const std::filesystem::path& direct_filename) const;
  ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger_;
};

#endif  // ANTARESXPANSION_LINKPROFILEREADER_H
