#ifndef ANTARESXPANSION_SRC_CPP_HELPERS_ARCHIVEUPDATER_H_
#define ANTARESXPANSION_SRC_CPP_HELPERS_ARCHIVEUPDATER_H_
#include <filesystem>

class ArchiveUpdater {
 public:
  ArchiveUpdater() = default;

  static void Update(const std::filesystem::path& antares_archive,
                     const std::filesystem::path& path_to_add,
                     const bool delete_path);
};
#endif  // ANTARESXPANSION_SRC_CPP_HELPERS_ARCHIVEUPDATER_H_
