//
// Created by marechaljas on 26/09/22.
//

#pragma once

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

class Adapter {
 public:
  explicit Adapter(const StudyFileReader::ILinkFileReader &link_file_reader);
  explicit Adapter(const StudyFileReader::IAreaFileReader &area_file_reader);
  explicit Adapter(
      const StudyFileReader::ICandidateFileReader &candidate_reader);
  Adapter(LinkFileReaderInMemory link_reader,
          AreaFileReaderInMemory area_reader,
          CandidateFileReaderInMemory candidate_reader);
  [[nodiscard]] std::vector<Link> Links(const std::string &study_path) const;
  [[nodiscard]] std::vector<Area> Areas(const std::string &study_path) const;
  std::vector<Candidate> Candidates(const std::string &study_path);
  XpansionStudy::Study Study(const std::string &study_path);
};
