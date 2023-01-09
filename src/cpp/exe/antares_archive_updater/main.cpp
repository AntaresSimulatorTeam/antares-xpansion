#include <iostream>

#include "ArchiveUpdater.h"
#include "ArchiveUpdaterExeOptions.h"

int main(int argc, char** argv) {
  ArchiveUpdaterExeOptions options_parser;

  options_parser.Parse(argc, argv);
  auto delete_path = options_parser.DeletePath();
  auto archive_path = options_parser.Archive();
  auto paths = options_parser.PathsToAdd();
  for (const auto& path : paths) {
    ArchiveUpdater::Update(archive_path, path, delete_path);
  }
  return 0;
}