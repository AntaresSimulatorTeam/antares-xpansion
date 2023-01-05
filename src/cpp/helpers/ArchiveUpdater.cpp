#include "ArchiveUpdater.h"

#include "ArchiveWriter.h"
void ArchiveUpdater::Update(const std::filesystem::path& archive_path,
                            const std::filesystem::path& path_to_add,
                            const bool delete_path) {
  auto archive_updater = ArchiveWriter(archive_path);
  archive_updater.Open();
  archive_updater.AddPathInArchive(path_to_add, path_to_add);
  archive_updater.Close();
  archive_updater.Delete();
  if (delete_path) {
    std::filesystem::remove_all(path_to_add);
  }
}