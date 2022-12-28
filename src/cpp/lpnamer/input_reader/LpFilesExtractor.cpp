#include "LpFilesExtractor.h"

#include "ArchiveReader.h"

void LpFilesExtractor::ExtractFiles() const {
  auto archive_reader = ArchiveReader(antares_archive_path_);
  archive_reader.Open();
  archive_reader.ExtractPattern((antares_archive_path_ / "area*.txt").string(),
                                "", xpansion_lp_dir_.parent_path());
  archive_reader.ExtractPattern(
      (antares_archive_path_ / "interco*.txt").string(), "",
      xpansion_lp_dir_.parent_path());
  archive_reader.ExtractPattern(
      (antares_archive_path_ / "ts-numbers*").string(), "",
      xpansion_lp_dir_.parent_path());
  archive_reader.Close();
  archive_reader.Delete();
}