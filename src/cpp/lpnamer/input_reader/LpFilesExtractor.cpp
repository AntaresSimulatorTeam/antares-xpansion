#include "LpFilesExtractor.h"

#include <sstream>

#include "ArchiveReader.h"

void LpFilesExtractor::ExtractFiles() const {
  auto archive_reader = ArchiveReader(antares_archive_path_);
  archive_reader.Open();
  const auto vect_area_files = archive_reader.ExtractPattern(
      (antares_archive_path_ / "area*.txt").string(), "",
      xpansion_lp_dir_.parent_path());

  auto num_areas_file = vect_area_files.size();
  if (num_areas_file == 0) {
    std::ostringstream msg;
    msg << "No area*.txt file found" << std::endl;
    (*logger_)(ProblemGenerationLog::LOGLEVEL::FATAL) << msg.str();
    throw ErrorWithAreaFile(msg.str());
  } else if (num_areas_file == 1) {
    std::ostringstream msg;
    msg << "More than one area*.txt file found" << std::endl;
    (*logger_)(ProblemGenerationLog::LOGLEVEL::FATAL) << msg.str();
    throw ErrorWithAreaFile(msg.str());
  }
  std::filesystem::rename(vect_area_files[0],
                          xpansion_lp_dir_.parent_path() / "area.txt");

  const auto vect_interco_files = archive_reader.ExtractPattern(
      (antares_archive_path_ / "interco*.txt").string(), "",
      xpansion_lp_dir_.parent_path());

  auto num_intercos_file = vect_interco_files.size();
  if (num_intercos_file == 0) {
    std::ostringstream msg;
    msg << "No interco*.txt file found" << std::endl;
    (*logger_)(ProblemGenerationLog::LOGLEVEL::FATAL) << msg.str();
    throw ErrorWithIntercosFile(msg.str());
  } else if (num_intercos_file == 1) {
    std::ostringstream msg;
    msg << "More than one interco*.txt file found" << std::endl;
    (*logger_)(ProblemGenerationLog::LOGLEVEL::FATAL) << msg.str();
    throw ErrorWithIntercosFile(msg.str());
  }
  std::filesystem::rename(vect_interco_files[0],
                          xpansion_lp_dir_.parent_path() / "interco.txt");
  archive_reader.ExtractPattern(
      (antares_archive_path_ / "ts-numbers*").string(), "",
      xpansion_lp_dir_.parent_path());
  archive_reader.Close();
  archive_reader.Delete();
}