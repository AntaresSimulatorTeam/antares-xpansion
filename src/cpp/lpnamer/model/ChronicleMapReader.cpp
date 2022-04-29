//
// Created by marechaljas on 28/04/2022.
//

#include "gtest/gtest.h"
#include "ChronicleMapReader.h"
#include <fstream>

std::map<unsigned int, unsigned int> ScenarioToChronicleReader::read(
    std::stringstream& input) const {
  ignoreFirstLine(input);
  AssignChronicleValuesToMCYears(input);

  return chronicle_map;
}
std::map<unsigned int, unsigned int> ScenarioToChronicleReader::ChronicleMap()
    const {
  return chronicle_map;
}
std::map<unsigned int, unsigned int> ScenarioToChronicleReader::read(
    const std::string& input) const {
  std::stringstream ss;
  ss << input;
  return  read(ss);
}
void ScenarioToChronicleReader::AssignChronicleValuesToMCYears(
    std::stringstream& ss) const {
  unsigned montecarlo_year = 1;
  while (!ss.eof()) {
    AssignChronicleValueToMCYear(ss, montecarlo_year);
    ++montecarlo_year;
  }
}
void ScenarioToChronicleReader::AssignChronicleValueToMCYear(
    std::stringstream& ss, unsigned int montecarlo_year) const {
  int value;
  ss >> value;
  if (!ss.fail()) {
    chronicle_map[montecarlo_year] = value;
  }
}
void ScenarioToChronicleReader::ignoreFirstLine(std::stringstream& ss) const {
  std::string garbage;
  std::getline(ss, garbage);
}
std::map<unsigned int, unsigned int> ScenarioToChronicleReader::read(
    const std::ifstream& input) const {
  std::stringstream ss;
  ss << input.rdbuf();
  return read(ss);
}
