//
// Created by marechaljas on 03/06/22.
//

#include <fstream>

#include "antares-xpansion/helpers/AntaresVersionProvider.h"
#include "gtest/gtest.h"


TEST(AntaresVersionProviderTest, defaultAntaresVersion) {
  AntaresVersionProvider antares_version_provider;
  ASSERT_EQ(antares_version_provider.getAntaresVersion("."), 710);
}

TEST(AntaresVersionProviderTest, antaresVersion700) {
  std::string content_l;
  std::ofstream file_l("study.antares");

  // dummy study tmp file name
  content_l =
      "\
[antares]\n\
version = 700\n\
caption = test_case_7.1_structure\n\
created = 1455269769\n\
lastsave = 1592911186\n\
author = Unknown\n\
";
  file_l << content_l;
  file_l.close();

  AntaresVersionProvider antares_version_provider;
  ASSERT_EQ(antares_version_provider.getAntaresVersion("."), 700);

  std::remove("study.antares");
}
