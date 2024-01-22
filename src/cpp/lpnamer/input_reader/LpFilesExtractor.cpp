#include "LpFilesExtractor.h"

#include <sstream>

#include "ArchiveReader.h"

void LpFilesExtractor::ExtractFiles() const {
  std::vector<std::filesystem::path> vect_area_files;
  std::vector<std::filesystem::path> vect_interco_files;

  if (!antares_archive_path_.empty()) {
    auto archive_reader = ArchiveReader(antares_archive_path_);
    archive_reader.Open();
    vect_area_files =
        archive_reader.ExtractPattern("area*.txt", "", xpansion_output_dir_);

    vect_interco_files =
        archive_reader.ExtractPattern("interco*.txt", "", xpansion_output_dir_);

    archive_reader.ExtractPattern("ts-numbers*", "", xpansion_output_dir_);
    archive_reader.Close();
    archive_reader.Delete();
  } else {
    for (auto &p : std::filesystem::directory_iterator(xpansion_output_dir_)) {
      if (p.path().extension() == ".txt") {
        if (p.path().filename().string().starts_with("area")) {
          vect_area_files.push_back(p.path());
        }
        if (p.path().filename().string().starts_with("interco")) {
          vect_interco_files.push_back(p.path());
        }
      }
    }
  }
  if (auto num_areas_file = vect_area_files.size(); num_areas_file == 0) {
    std::ostringstream msg;
    auto log_location = LOGLOCATION;
    msg << "No area*.txt file found" << std::endl;
    (*logger_)(LogUtils::LOGLEVEL::FATAL)
        << log_location << msg.str();
    throw ErrorWithAreaFile(msg.str(), log_location);
  } else if (num_areas_file > 1) {
    std::ostringstream msg;
    auto log_location = LOGLOCATION;
    msg << "More than one area*.txt file found" << std::endl;
    (*logger_)(LogUtils::LOGLEVEL::FATAL)
        << log_location << msg.str();
    throw ErrorWithAreaFile(msg.str(), log_location);
  }
  std::filesystem::rename(vect_area_files[0],
                          xpansion_output_dir_ / "area.txt");

  if (auto num_intercos_file = vect_interco_files.size();
      num_intercos_file == 0) {
    std::ostringstream msg;
    msg << "No interco*.txt file found" << std::endl;
    auto log_location = LOGLOCATION;
    (*logger_)(LogUtils::LOGLEVEL::FATAL)
        << log_location << msg.str();
    throw ErrorWithIntercosFile(msg.str(), log_location);
  } else if (num_intercos_file > 1) {
    std::ostringstream msg;
    auto log_location = LOGLOCATION;
    msg << "More than one interco*.txt file found" << std::endl;
    (*logger_)(LogUtils::LOGLEVEL::FATAL)
        << log_location << msg.str();
    throw ErrorWithIntercosFile(msg.str(), log_location);
  }
  std::filesystem::rename(vect_interco_files[0],
                          xpansion_output_dir_ / "interco.txt");
}