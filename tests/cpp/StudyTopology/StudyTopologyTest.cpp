//
// Created by marechaljas on 20/09/22.
//
#include <filesystem>

#include "CoreHexagone/ForProvidingXpansionStudy.h"
#include "ForProvidingXpansionStudyInMemoryAdapter.h"
#include "gtest/gtest.h"

TEST(StudyTopology, Study_noLink) {
  //Given a study
    //1 lien sans candidat
  // When I start the study
  // Then produce nothing

  auto study_adapter = std::make_shared<ForProvidingXpansionStudyInMemoryAdapter>();
  ForProvidingXpansionStudy link_service(study_adapter);
  std::filesystem::path study;
  XpansionStudy xpansion_study = link_service.provide(study);
  EXPECT_TRUE(xpansion_study.Links().empty());
}

TEST(StudyTopology, IgnoreLinkWithoutCandidates) {
  //Given a study
  //1 lien avec candidat
  // When I start the study
  // Then produce nothing

  auto study_adapter = std::make_shared<ForProvidingXpansionStudyInMemoryAdapter>();
  study_adapter->addLink({});
  ForProvidingXpansionStudy providing_xpansion_study(study_adapter);
  std::filesystem::path study;
  XpansionStudy xpansion_study = providing_xpansion_study.provide(study);
  EXPECT_EQ(xpansion_study.Links().size(), 0);
}