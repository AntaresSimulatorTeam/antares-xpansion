//
// Created by marechaljas on 26/09/22.
//

#include <gtest/gtest.h>

#include "ACL/LinkFromFileTranslator.h"
#include "ACL/XpansionStudyFileAdapter.h"
#include "CoreHexagone/Model/Candidate.h"
#include "CoreHexagone/Model/Link.h"
#include "CoreHexagone/Model/Study.h"
#include "XpansionStudyFileReader/Stub/AreaFileReaderInMemory.h"
#include "XpansionStudyFileReader/Stub/CandidateFileReaderInMemory.h"
#include "XpansionStudyFileReader/Stub/LinkFileReaderInMemory.h"

TEST(ACL, NoLinkForEmptyFile) {
  //Read empty link

  LinkFileReaderInMemory link_reader;

  XpansionStudyFileAdapter adapter(link_reader);
  std::vector<XpansionStudy::Link> links = adapter.Links("StudyPath");
  EXPECT_TRUE(links.empty());
}

TEST(ACL, NoAreaForEmptyFile) {
  AreaFileReaderInMemory area_file_reader;

  XpansionStudyFileAdapter adapter(area_file_reader);
  std::vector<XpansionStudy::Area> areas = adapter.Areas("StudyPath");
  EXPECT_TRUE(areas.empty());
}

TEST(ACL, NoCandidatesForEmptyFile) {
  CandidateFileReaderInMemory candidate_file_reader;

  XpansionStudyFileAdapter adapter(candidate_file_reader);
  std::vector<XpansionStudy::Candidate> candidates = adapter.Candidates("StudyPath");
  EXPECT_TRUE(candidates.empty());
}

TEST(ACL, ProvidefullStudy) {
  auto link_reader = std::make_shared<LinkFileReaderInMemory>();
  auto area_file_reader = std::make_shared<AreaFileReaderInMemory>();
  auto candidate_file_reader = std::make_shared<CandidateFileReaderInMemory>();

  auto link_translator = std::make_shared<LinkFromFileTranslator>();
  XpansionStudyFileAdapter adapter(link_reader, area_file_reader, candidate_file_reader, link_translator);
  XpansionStudy::Study study = adapter.Study("StudyPath");
  EXPECT_TRUE(study.Areas().empty());
  EXPECT_TRUE(study.Links().empty());
  EXPECT_TRUE(study.Candidates().empty());
}

TEST(ACL, LinkWithCandidate) {
  auto link_reader = std::make_shared<LinkFileReaderInMemory>();
  auto area_file_reader = std::make_shared<AreaFileReaderInMemory>();
  auto candidate_file_reader = std::make_shared<CandidateFileReaderInMemory>();

  StudyFileReader::Candidate candidate_input;
  candidate_file_reader->Feed(candidate_input);
  StudyFileReader::Link link_input;
  link_reader->Feed(link_input);

  auto link_translator = std::make_shared<LinkFromFileTranslator>();

  XpansionStudyFileAdapter adapter(link_reader, area_file_reader, candidate_file_reader, link_translator);
  XpansionStudy::Study study = adapter.Study("StudyPath");

  const XpansionStudy::Link link = study.Links().front();
  const XpansionStudy::Candidate candidate = link.Candidates().front();
  auto expected_candidate = link_translator->translate(std::vector<StudyFileReader::Link>{link_input}, {candidate_input}).front().Candidates().front();
  EXPECT_EQ(candidate, expected_candidate);
}