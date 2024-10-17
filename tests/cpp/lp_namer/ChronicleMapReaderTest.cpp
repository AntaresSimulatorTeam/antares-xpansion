//
// Created by marechaljas on 27/04/2022.
//
#include <fstream>
#include "gtest/gtest.h"
#include "antares-xpansion/lpnamer/model/ChronicleMapReader.h"

class ChronicleTest: public ::testing::Test {
 public:
  ScenarioToChronicleReader reader_;

 protected:
  void SetUp() override {

  }
};

TEST_F(ChronicleTest, InputContainsOneChroniclesValues42ThenMCY1Give42) {
  auto text = "Garbage\n42";

  ASSERT_EQ(reader_.read(text).at(1), 42);
}

TEST_F(ChronicleTest, InputContainsTwoChroniclesValuesThenMCYMatch) {
  auto text = "Garbage\n42\n32";

  ASSERT_EQ(reader_.read(text).at(1), 42);
  ASSERT_EQ(reader_.read(text).at(2), 32);
}

TEST_F(ChronicleTest, IgnoreFirstLine) {
  auto text = "42";

  ASSERT_NE(reader_.read(text).size(), 1);
}

TEST_F(ChronicleTest, AcceptStreamAsInput) {
  auto text = "Garbage\n42\n32";
  std::stringstream ss;
  ss << text;

  ASSERT_EQ(reader_.read(ss).at(1), 42);
  ASSERT_EQ(reader_.read(ss).at(2), 32);
}

TEST_F(ChronicleTest, AcceptFileAsInput) {
  auto temp_name = std::tmpnam(nullptr);
  std::ofstream file(temp_name);
  file << "Garbage\n42\n52\n";
  file.close();
  std::ifstream ifile(temp_name);
  ASSERT_EQ(reader_.read(ifile).at(1), 42);
}

TEST_F(ChronicleTest, ProperlyHandleLAstLineEnding) {
  auto temp_name = std::tmpnam(nullptr);
  std::ofstream file(temp_name);
  file << "Garbage\n42\n52\n";
  file.close();
  std::ifstream ifile(temp_name);
  std::map<unsigned, unsigned > expected = {{1,42}, {2,52}};
  ASSERT_EQ(reader_.read(ifile), expected);
}