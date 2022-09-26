//
// Created by marechaljas on 26/09/22.
//

#include <gtest/gtest.h>

#include "ACL/Adapter.h"
#include "CoreHexagone/Link.h"
#include "StudyFileAdapter/ILinkFileReader.h"
#include "StudyFileAdapter/Stub/LinkFileReaderInMemory.h"

TEST(ACL, Foo) {
  //Read empty link

  LinkFileReaderInMemory link_reader;

  Adapter adapter(link_reader);
  std::vector<Link> links = adapter.Links("StudyPath");
  EXPECT_EQ(links.size(), 0);
}

