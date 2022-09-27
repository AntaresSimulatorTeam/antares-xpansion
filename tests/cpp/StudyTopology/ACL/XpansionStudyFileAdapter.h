//
// Created by marechaljas on 26/09/22.
//

#pragma once

#include <memory>

#include "../CoreHexagone/IForProvidingStudyPort.h"
#include "../CoreHexagone/Model/Area.h"
#include "../CoreHexagone/Model/Candidate.h"
#include "../CoreHexagone/Model/Link.h"
#include "../CoreHexagone/Model/Study.h"
#include "../XpansionStudyFileReader/IAreaFileReader.h"
#include "../XpansionStudyFileReader/ICandidateFileReader.h"
#include "../XpansionStudyFileReader/ILinkFileReader.h"
#include "../XpansionStudyFileReader/Stub/AreaFileReaderInMemory.h"
#include "../XpansionStudyFileReader/Stub/CandidateFileReaderInMemory.h"
#include "../XpansionStudyFileReader/Stub/LinkFileReaderInMemory.h"
#include "ILinkTranslator.h"

class XpansionStudyFileAdapter: public IForProvidingStudyPort {
 public:
  explicit XpansionStudyFileAdapter(const StudyFileReader::ILinkFileReader &link_file_reader);
  explicit XpansionStudyFileAdapter(const StudyFileReader::IAreaFileReader &area_file_reader);
  explicit XpansionStudyFileAdapter(
      const StudyFileReader::ICandidateFileReader &candidate_reader);

  XpansionStudyFileAdapter(std::shared_ptr<StudyFileReader::ILinkFileReader> link_reader,
          std::shared_ptr<StudyFileReader::IAreaFileReader> area_reader,
          std::shared_ptr<StudyFileReader::ICandidateFileReader> candidate_reader,
          std::shared_ptr<ILinkTranslator> link_translator);
  [[nodiscard]] std::vector<XpansionStudy::Link> Links(const std::string &study_path) const;
  [[nodiscard]] std::vector<XpansionStudy::Area> Areas(const std::string &study_path) const;
  std::vector<XpansionStudy::Candidate> Candidates(const std::string &study_path);
  [[nodiscard]] XpansionStudy::Study Study(const std::string &study_path) const;

 private:
  std::shared_ptr<StudyFileReader::ILinkFileReader> link_reader_;
  std::shared_ptr<StudyFileReader::IAreaFileReader> area_reader_;
  std::shared_ptr<StudyFileReader::ICandidateFileReader> candidate_reader_;
  std::shared_ptr<ILinkTranslator> link_translator_;
  void removeOrphanLinks(std::vector<XpansionStudy::Link>&links) const;
};
