//
// Created by marechaljas on 29/04/2022.
//

#include "ChronicleMapReader.h"
#include <utility>
#include <fstream>
#include <filesystem>
#include <gtest/gtest.h>
#include "ChronicleMapProvider.h"
std::map<unsigned int, unsigned int>
DirectAccessScenarioToChronicleProvider::GetMap(
    const std::string& link_from, const std::string& link_to) const {
  std::filesystem::path file_path = GetPath(link_from, link_to);

  std::ifstream file(file_path);
  /* We could check antares custom-ts-numbers to see if there is a scenario
   * however this would mean reading antares configuration files which is not
   * expected at the moment
   * */
  if (!file.is_open()) {
    std::cout << "No scenario builder output found for link at destination: " << file_path << std::endl;
    return {};
  }
  return chronicle_map_reader_.read(file);
}

std::filesystem::path DirectAccessScenarioToChronicleProvider::GetPath(
    const std::string& link_from, const std::string& link_to) const {
  std::filesystem::path file_path;
  if (link_from < link_to) {
    file_path = (ts_info_root_ / link_from / link_to);
  } else {
    file_path = (ts_info_root_ / link_to / link_from);
  }
  return file_path;
}
