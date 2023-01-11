#ifndef ANTARESXPANSION_SRC_CPP_HELPERS_ARCHIVEUPDATER_H_
#define ANTARESXPANSION_SRC_CPP_HELPERS_ARCHIVEUPDATER_H_
#include <filesystem>
#include <vector>

#include "ArchiveWriter.h"

class ArchiveUpdater {
 public:
  ArchiveUpdater() = default;

  static void Update(ArchiveWriter& writer,
                     const std::filesystem::path& path_to_add,
                     const bool delete_path);
  static void CleanAntaresArchive(
      const std::filesystem::path& src_archive,
      const std::filesystem::path& target_archive = "");
  static int32_t MinizipErase(void* reader, void* writer);
};
#endif  // ANTARESXPANSION_SRC_CPP_HELPERS_ARCHIVEUPDATER_H_
