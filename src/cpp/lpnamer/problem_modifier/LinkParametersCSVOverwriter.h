//
// Created by marechaljas on 16/06/22.
//

#pragma once

#include <filesystem>

#include "LinkdataRecord.h"

class LinkParametersCSVOverWriter {
 public:
  LinkParametersCSVOverWriter(
      ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger)
      : logger_(std::move(logger)) {}

  bool open(const std::filesystem::path& linkdataFilename_l);

  void commit();

  void WriteRow(LinkdataRecord const& record);

  void FillRecord(std::string basic_string_1, LinkdataRecord& record);

 private:
  std::filesystem::path link_parameters_file_path_;
  std::filesystem::path tempOutCsvFile_name_;
  std::ifstream inputCsv_l_;
  std::ofstream tempOutCsvFile;
  bool already_warned_ = false;
  ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger_;
};
