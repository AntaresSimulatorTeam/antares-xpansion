#include <iostream>

#include "ArchiveUpdater.h"
#include "ArchiveUpdaterExeOptions.h"
#include "ArchiveWriter.h"

int main(int argc, char** argv) {
  ArchiveUpdaterExeOptions options_parser;

  options_parser.Parse(argc, argv);
  auto delete_path = options_parser.DeletePath();
  auto archive_path = options_parser.Archive();
  auto paths = options_parser.PathsToAdd();
  auto writer = ArchiveWriter(archive_path);
  writer.Open();
  for (const auto& path : paths) {
    ArchiveUpdater::Update(writer, path, delete_path);
  }
  writer.Close();
  writer.Delete();
  ArchiveUpdater::DeleteFromArchive(archive_path);

  // std::filesystem::remove_all()
  return 0;
}