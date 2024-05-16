#include "LpFilesExtractor.h"

#include <algorithm>
#include <sstream>

#include "ArchiveReader.h"

void LpFilesExtractor::ExtractFiles() const {
  auto [vect_area_files, vect_interco_files] = getFiles();
  if (auto num_areas_file = vect_area_files.size(); num_areas_file == 0) {
    std::ostringstream msg;
    auto log_location = LOGLOCATION;
    msg << "No area*.txt file found" << std::endl;
    (*logger_)(LogUtils::LOGLEVEL::FATAL) << log_location << msg.str();
    throw ErrorWithAreaFile(msg.str(), log_location);
  } else if (num_areas_file > 1) {
    std::ostringstream msg;
    auto log_location = LOGLOCATION;
    msg << "More than one area*.txt file found" << std::endl;
    (*logger_)(LogUtils::LOGLEVEL::FATAL) << log_location << msg.str();
    throw ErrorWithAreaFile(msg.str(), log_location);
  }

  switch (mode_) {
    case Mode::ANTARES_API:
      [[fallthrough]];
    case Mode::FILE:
      try {
        std::filesystem::copy(vect_area_files[0],
                              xpansion_output_dir_ / "area.txt");
      } catch (const std::filesystem::filesystem_error& e) {
        auto log_location = LOGLOCATION;
        (*logger_)(LogUtils::LOGLEVEL::FATAL) << log_location << e.what();
        throw;
      }
      break;
    case Mode::ARCHIVE:
      std::filesystem::rename(vect_area_files[0],
                              xpansion_output_dir_ / "area.txt");
      break;
    case Mode::UNKOWN:
      throw LogUtils::XpansionError<std::runtime_error>(
          "Mode is unknown", LOGLOCATION);
    default:
      throw LogUtils::XpansionError<std::runtime_error>(
          "Mode is not supported:", LOGLOCATION);
  }

  if (auto num_intercos_file = vect_interco_files.size();
      num_intercos_file == 0) {
    std::ostringstream msg;
    msg << "No interco*.txt file found" << std::endl;
    auto log_location = LOGLOCATION;
    (*logger_)(LogUtils::LOGLEVEL::FATAL) << log_location << msg.str();
    throw ErrorWithIntercosFile(msg.str(), log_location);
  } else if (num_intercos_file > 1) {
    std::ostringstream msg;
    auto log_location = LOGLOCATION;
    msg << "More than one interco*.txt file found" << std::endl;
    (*logger_)(LogUtils::LOGLEVEL::FATAL) << log_location << msg.str();
    throw ErrorWithIntercosFile(msg.str(), log_location);
  }
  switch (mode_) {
    case Mode::ANTARES_API:
      [[fallthrough]];
    case Mode::FILE:
      try {
        std::filesystem::copy(vect_interco_files[0],
                              xpansion_output_dir_ / "interco.txt");
      } catch (const std::filesystem::filesystem_error& e) {
        auto log_location = LOGLOCATION;
        (*logger_)(LogUtils::LOGLEVEL::FATAL) << log_location << e.what();
        throw;
      }
      break;
    case Mode::ARCHIVE:
      std::filesystem::rename(vect_interco_files[0],
                              xpansion_output_dir_ / "interco.txt");
      break;
    case Mode::UNKOWN:
      throw LogUtils::XpansionError<std::runtime_error>(
          "Mode is unknown", LOGLOCATION);
    default:
      throw LogUtils::XpansionError<std::runtime_error>(
          "Mode is not supported:", LOGLOCATION);
  }
}
LpFilesExtractor::areaAndIntecoPaths LpFilesExtractor::getFiles() const{
  std::vector<std::filesystem::path> vect_area_files;
  std::vector<std::filesystem::path> vect_interco_files;
  switch (this->mode_) {
    case Mode::ARCHIVE: {
      return getFilesFromArchive();
      break;
    }
    case Mode::ANTARES_API:
      [[fallthrough]];
    case Mode::FILE: {
      auto dit = std::filesystem::directory_iterator(this->simulation_dir_);
      std::ranges::for_each(
          dit, [&vect_area_files, &vect_interco_files](const auto& entry) {
            if (entry.path().extension() == ".txt") {
              if (entry.path().filename().string().starts_with("area")) {
                vect_area_files.push_back(entry.path());
              }
              if (entry.path().filename().string().starts_with("interco")) {
                vect_interco_files.push_back(entry.path());
              }
            }
          });
    } break;
    case Mode::UNKOWN:
      throw LogUtils::XpansionError<std::runtime_error>(
          "Mode is unknown", LOGLOCATION);
    default:
      throw LogUtils::XpansionError<std::runtime_error>(
          "Mode is not supported:", LOGLOCATION);
  }
  return std::make_pair(vect_area_files, vect_interco_files);
}
LpFilesExtractor::areaAndIntecoPaths LpFilesExtractor::getFilesFromArchive() const {
  std::vector<std::filesystem::path> vect_area_files;
  std::vector<std::filesystem::path> vect_interco_files;
  auto archive_reader = ArchiveReader(this->antares_archive_path_);
  archive_reader.Open();
  vect_area_files =
      archive_reader.ExtractPattern("area*.txt", "", this->xpansion_output_dir_);

  vect_interco_files = archive_reader.ExtractPattern("interco*.txt", "",
                                                     this->xpansion_output_dir_);

  archive_reader.ExtractPattern("ts-numbers*", "", this->xpansion_output_dir_);
  archive_reader.Close();
  archive_reader.Delete();
  return std::make_pair(vect_area_files, vect_interco_files);
}
