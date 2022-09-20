//
// Created by marechaljas on 20/09/22.
//
#include <filesystem>

#include "Link.h"
#include "LinkService.h"
#include "gtest/gtest.h"

TEST(Foo, bar) {
  //Given a study
    //1 candidat sur un lien reliant 2 zones
  //When I start the study
  //Then produce a model with 1 link associated with 1 candidate
}

TEST(Foo, bar1) {
  //Given a study
    //1 lien sans candidat
  // When I start the study
  // Then produce nothing

  LinkService link_service;
  std::filesystem::path study;
  std::vector<Link> link_list = link_service.Load(study);
  EXPECT_TRUE(link_list.empty());
}