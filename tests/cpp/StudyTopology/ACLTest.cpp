//
// Created by marechaljas on 26/09/22.
//

#include <gtest/gtest.h>

#include "ACL/Adapter.h"
#include "CoreHexagone/Candidate.h"
#include "CoreHexagone/Link.h"
#include "CoreHexagone/Study.h"
#include "StudyFileAdapter/ILinkFileReader.h"
#include "StudyFileAdapter/Stub/AreaFileReaderInMemory.h"
#include "StudyFileAdapter/Stub/CandidateFileReaderInMemory.h"
#include "StudyFileAdapter/Stub/LinkFileReaderInMemory.h"

TEST(ACL, NoLinkForEmptyFile) {
  //Read empty link

  LinkFileReaderInMemory link_reader;

  Adapter adapter(link_reader);
  std::vector<Link> links = adapter.Links("StudyPath");
  EXPECT_TRUE(links.empty());
}

TEST(ACL, NoAreaForEmptyFile) {
  AreaFileReaderInMemory area_file_reader;

  Adapter adapter(area_file_reader);
  std::vector<Area> areas = adapter.Areas("StudyPath");
  EXPECT_TRUE(areas.empty());
}

TEST(ACL, NoCandidatesForEmptyFile) {
  CandidateFileReaderInMemory candidate_file_reader;

  Adapter adapter(candidate_file_reader);
  std::vector<Candidate> candidates = adapter.Candidates("StudyPath");
  EXPECT_TRUE(candidates.empty());
}

TEST(ACL, ProvidefullStudy) {
  LinkFileReaderInMemory link_reader;
  AreaFileReaderInMemory area_file_reader;
  CandidateFileReaderInMemory candidate_file_reader;

  Adapter adapter(link_reader, area_file_reader, candidate_file_reader);
  XpansionStudy::Study study = adapter.Study("StudyPath");
  EXPECT_TRUE(study.Areas().empty());
  EXPECT_TRUE(study.Links().empty());
  EXPECT_TRUE(study.Candidates().empty());
}