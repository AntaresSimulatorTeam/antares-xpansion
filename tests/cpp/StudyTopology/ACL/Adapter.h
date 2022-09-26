//
// Created by marechaljas on 26/09/22.
//

#pragma once

#include <memory>

#include "../CoreHexagone/Area.h"
#include "../CoreHexagone/Candidate.h"
#include "../CoreHexagone/Link.h"
#include "../CoreHexagone/Study.h"
#include "../StudyFileAdapter/IAreaFileReader.h"
#include "../StudyFileAdapter/ICandidateFileReader.h"
#include "../StudyFileAdapter/ILinkFileReader.h"
#include "../StudyFileAdapter/Stub/AreaFileReaderInMemory.h"
#include "../StudyFileAdapter/Stub/CandidateFileReaderInMemory.h"
#include "../StudyFileAdapter/Stub/LinkFileReaderInMemory.h"
#include "ILinkTranslator.h"

class Adapter {
 public:
  explicit Adapter(const StudyFileReader::ILinkFileReader &link_file_reader);
  explicit Adapter(const StudyFileReader::IAreaFileReader &area_file_reader);
  explicit Adapter(
      const StudyFileReader::ICandidateFileReader &candidate_reader);

  Adapter(std::shared_ptr<StudyFileReader::ILinkFileReader> link_reader,
          std::shared_ptr<StudyFileReader::IAreaFileReader> area_reader,
          std::shared_ptr<StudyFileReader::ICandidateFileReader> candidate_reader,
          std::shared_ptr<ILinkTranslator> link_translator);
  [[nodiscard]] std::vector<XpansionStudy::Link> Links(const std::string &study_path) const;
  [[nodiscard]] std::vector<XpansionStudy::Area> Areas(const std::string &study_path) const;
  std::vector<XpansionStudy::Candidate> Candidates(const std::string &study_path);
  XpansionStudy::Study Study(const std::string &study_path);

 private:
  std::shared_ptr<StudyFileReader::ILinkFileReader> link_reader_;
  std::shared_ptr<StudyFileReader::IAreaFileReader> area_reader_;
  std::shared_ptr<StudyFileReader::ICandidateFileReader> candidate_reader_;
  std::shared_ptr<ILinkTranslator> link_translator_;
};
