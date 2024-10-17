//
// Created by marechaljas on 16/06/22.
//

#include "antares-xpansion/study-updater/LinkParametersCSVOverwriter.h"

#include "JsonXpansionReader.h"
#include "antares-xpansion/lpnamer/problem_modifier/LinkdataRecord.h"

bool LinkParametersCSVOverWriter::open(
    const std::filesystem::path& linkdataFilename_l) {
  link_parameters_file_path_ = linkdataFilename_l;
  inputCsv_l_ = std::ifstream(linkdataFilename_l);
  tempOutCsvFile_name_ = linkdataFilename_l.string() + "tmp";
  tempOutCsvFile = std::ofstream(tempOutCsvFile_name_);

  if (!tempOutCsvFile.is_open()) {
    (*logger_)(LogUtils::LOGLEVEL::ERR)
        << "ERROR: Error writing " + linkdataFilename_l.string() + "."
        << std::endl;
    return false;
  }

  already_warned_ = false;
  if (!inputCsv_l_.is_open()) {
    (*logger_)(LogUtils::LOGLEVEL::WARNING)
        << "WARNING: Missing file to update antares study : "
        << linkdataFilename_l << "."
        << " Unknown values were populated with 0 in a newly created file."
        << std::endl;
    already_warned_ = true;
  }
  return true;
}
void LinkParametersCSVOverWriter::commit() {
  inputCsv_l_.close();
  tempOutCsvFile.close();

  // delete old file and rename the temporarily created file
  std::filesystem::remove(link_parameters_file_path_);
  std::filesystem::rename(tempOutCsvFile_name_, link_parameters_file_path_);
}
void LinkParametersCSVOverWriter::WriteRow(const LinkdataRecord& record) {
  tempOutCsvFile << record.to_row("\t") << "\n";
}
void LinkParametersCSVOverWriter::FillRecord(std::string basic_string_1,
                                             LinkdataRecord& record) {
  if (std::getline(inputCsv_l_, basic_string_1)) {
    record.fillFromRow(basic_string_1);
  } else {
    record.reset();
    if (!already_warned_) {
      (*logger_)(LogUtils::LOGLEVEL::WARNING)
          << "WARNING: Missing entries in : "
          << link_parameters_file_path_.string()
          << ". Missing values were populated with 0." << std::endl;
      already_warned_ = true;
    }
  }
}
