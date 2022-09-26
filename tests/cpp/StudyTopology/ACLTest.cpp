//
// Created by marechaljas on 26/09/22.
//

#include <gtest/gtest.h>

#include "ACL/Adapter.h"
#include "AreaFileReaderInMemory.h"
#include "CandidateFileReaderInMemory.h"
#include "CoreHexagone/Candidate.h"
#include "CoreHexagone/Link.h"
#include "StudyFileAdapter/ILinkFileReader.h"
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