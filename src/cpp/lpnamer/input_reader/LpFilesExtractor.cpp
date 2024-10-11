#include "LpFilesExtractor.h"

#include <algorithm>
#include <sstream>

#include "antares-xpansion/helpers/ArchiveReader.h"

void LpFilesExtractor::ExtractFiles() const {
  auto [vect_area_files, vect_interco_files] = getFiles();

  checkProperNumberOfAreaFiles(vect_area_files);
  produceAreatxtFile(vect_area_files);

  checkProperNumberOfIntercoFiles(vect_interco_files);
  produceIntercotxtFile(vect_interco_files);
}

/**
 * @brief This method is used to get the files from the simulation directory.
 *
 * The patterns it looks for are "area*.txt" and "interco*.txt".
 *
 * @return A pair of vectors containing the paths of the files. The first vector contains the paths of the "area*.txt" files and the second vector contains the paths of the "interco*.txt" files.
 */
LpFilesExtractor::areaAndIntercoPaths LpFilesExtractor::getFiles() const{
  std::vector<std::filesystem::path> vect_area_files;
  std::vector<std::filesystem::path> vect_interco_files;
  switch (this->mode_) {
    case SimulationInputMode::ARCHIVE: {
      return getFilesFromArchive();
    }
    case SimulationInputMode::ANTARES_API:
      [[fallthrough]];
    case SimulationInputMode::FILE: {
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
    default:
      throw LogUtils::XpansionError<std::runtime_error>(
          "SimulationInputMode is not supported:", LOGLOCATION);
  }
  return std::pair(vect_area_files, vect_interco_files);
}

/**
 * @brief This method is used to extract files from an archive.
 *
 * The patterns it looks for are "area*.txt", "interco*.txt", and "ts-numbers*". The extracted files are stored in the `xpansion_output_dir_` directory.
 *
 * @return A pair of vectors containing the paths of the extracted files. The first vector contains the paths of the "area*.txt" files and the second vector contains the paths of the "interco*.txt" files.
 */
LpFilesExtractor::areaAndIntercoPaths LpFilesExtractor::getFilesFromArchive() const {
  std::vector<std::filesystem::path> vect_area_files;
  std::vector<std::filesystem::path> vect_interco_files;
  auto archive_reader = ArchiveReader(this->antares_archive_path_);
  archive_reader.Open();
  vect_area_files =
      archive_reader.ExtractPattern("area*.txt", "", this->xpansion_output_dir_);

  vect_interco_files = archive_reader.ExtractPattern("interco*.txt", "",
                                                     this->xpansion_output_dir_);

  archive_reader.ExtractPattern("ts-numbers*", "", this->xpansion_output_dir_);
  return std::pair(vect_area_files, vect_interco_files);
}


/**
 * @brief This method checks if the number of area files is correct.
 *
 * We should have only one file named "area*.txt".
 *
 * @param vect_area_files
 */
void LpFilesExtractor::checkProperNumberOfAreaFiles(
    const std::vector<std::filesystem::path>& vect_area_files) const {
  if (auto num_areas_file = vect_area_files.size(); num_areas_file == 0) {
    std::ostringstream msg;
    auto log_location = LOGLOCATION;
    msg << "No area*.txt file found" << std::endl;
    (*this->logger_)(LogUtils::LOGLEVEL::FATAL) << log_location << msg.str();
    throw ErrorWithAreaFile(msg.str(), log_location);
  } else if (num_areas_file > 1) {
    std::ostringstream msg;
    auto log_location = LOGLOCATION;
    msg << "More than one area*.txt file found" << std::endl;
    (*this->logger_)(LogUtils::LOGLEVEL::FATAL) << log_location << msg.str();
    throw ErrorWithAreaFile(msg.str(), log_location);
  }
}

/**
 * @brief This method is used to produce the "area.txt" file from "area*.txt"
 *
 * In archive mode the file is just rename. Otherwise, the file is copied from
 * the simulation directory to the `xpansion_output_dir_` directory.
 *
 * @param vect_area_files
 */
void LpFilesExtractor::produceAreatxtFile(
    const std::vector<std::filesystem::path>& vect_area_files) const {
  switch (this->mode_) {
    case SimulationInputMode::ANTARES_API:
      [[fallthrough]];
    case SimulationInputMode::FILE:
      try {
        std::filesystem::rename(vect_area_files[0],
                              this->xpansion_output_dir_ / "area.txt");
      } catch (const std::filesystem::filesystem_error& e) {
        auto log_location = LOGLOCATION;
        (*this->logger_)(LogUtils::LOGLEVEL::FATAL) << log_location << e.what();
        throw;
      }
      break;
    case SimulationInputMode::ARCHIVE:
      std::filesystem::rename(vect_area_files[0],
                              this->xpansion_output_dir_ / "area.txt");
      break;
    default:
      throw LogUtils::XpansionError<std::runtime_error>(
          "SimulationInputMode is not supported:", LOGLOCATION);
  }
}

/**
 * @brief This method checks if the number of interco files is correct.
 *
 * We should have only one file named "interco*.txt".
 *
 * @param vect_interco_files
 */
void LpFilesExtractor::checkProperNumberOfIntercoFiles(
    const std::vector<std::filesystem::path>& vect_interco_files) const {
  if (auto num_interco_file = vect_interco_files.size();
      num_interco_file == 0) {
    std::ostringstream msg;
    msg << "No interco*.txt file found" << std::endl;
    auto log_location = LOGLOCATION;
    (*this->logger_)(LogUtils::LOGLEVEL::FATAL) << log_location << msg.str();
    throw ErrorWithIntercosFile(msg.str(), log_location);
  } else if (num_interco_file > 1) {
    std::ostringstream msg;
    auto log_location = LOGLOCATION;
    msg << "More than one interco*.txt file found" << std::endl;
    (*this->logger_)(LogUtils::LOGLEVEL::FATAL) << log_location << msg.str();
    throw ErrorWithIntercosFile(msg.str(), log_location);
  }
}

/**
 * @brief This method is used to produce the "interco.txt" file from "interco*.txt"
 *
 * In archive mode the file is just rename. Otherwise, the file is copied from
 * the simulation directory to the `xpansion_output_dir_` directory.
 *
 * @param vect_interco_files
 */
void LpFilesExtractor::produceIntercotxtFile(
    const std::vector<std::filesystem::path>& vect_interco_files) const {
  switch (this->mode_) {
    case SimulationInputMode::ANTARES_API:
      [[fallthrough]];
    case SimulationInputMode::FILE:
      try {
        std::filesystem::rename(vect_interco_files[0],
                              this->xpansion_output_dir_ / "interco.txt");
      } catch (const std::filesystem::filesystem_error& e) {
        auto log_location = LOGLOCATION;
        (*this->logger_)(LogUtils::LOGLEVEL::FATAL) << log_location << e.what();
        throw;
      }
      break;
    case SimulationInputMode::ARCHIVE:
      std::filesystem::rename(vect_interco_files[0],
                              this->xpansion_output_dir_ / "interco.txt");
      break;
    default:
      throw LogUtils::XpansionError<std::runtime_error>(
          "SimulationInputMode is not supported:", LOGLOCATION);
  }
}
