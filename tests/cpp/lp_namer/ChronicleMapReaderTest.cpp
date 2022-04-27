//
// Created by marechaljas on 27/04/2022.
//
#include "gtest/gtest.h"

class ScenarioToChronicleReader {
 private:
  std::map<unsigned, unsigned > chronicle_map;

  void ignoreFirstLine(std::stringstream& ss) const {
    std::string garbage;
    std::getline(ss, garbage);
  }

  void AssignChronicleValueToMCYear(std::stringstream& ss,
                                    unsigned int montecarlo_year) {
    int value;
    ss >> value;
    chronicle_map[montecarlo_year] = value;
  }
  void AssignChronicleValuesToMCYears(std::stringstream& ss) {
    unsigned montecarlo_year = 1;
    while (!ss.eof()) {
      AssignChronicleValueToMCYear(ss, montecarlo_year);
      ++montecarlo_year;
    }
  }

 public:
  [[nodiscard]] std::map<unsigned int, unsigned int> read(std::string const& input) {
    std::stringstream ss;
    ss << input;
    return  read(ss);
  }

  [[nodiscard]] std::map<unsigned int, unsigned int> read(std::stringstream& input) {
    ignoreFirstLine(input);
    AssignChronicleValuesToMCYears(input);
    return chronicle_map;
  }

  [[nodiscard]] std::map<unsigned int, unsigned int> ChronicleMap() const {
    return chronicle_map;
  }


};

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

