#include "ArchiveUpdater.h"

#include "ArchiveWriter.h"
void ArchiveUpdater::Update(const std::filesystem::path& archive_path,
                            const std::filesystem::path& path_to_add,
                            const bool delete_path) {
  auto archive_updater = ArchiveWriter(archive_path);
  archive_updater.Open();
  auto root_path = path_to_add;
  // if (!std::filesystem::is_directory(path_to_add)) {
  root_path = path_to_add.parent_path();
  // }
  archive_updater.AddPathInArchive(path_to_add, root_path);
  archive_updater.Close();
  archive_updater.Delete();
  if (delete_path) {
    std::filesystem::remove_all(path_to_add);
  }
}