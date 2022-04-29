//
// Created by marechaljas on 02/05/2022.
//

#include <filesystem>
#include <cstring>
#include "ProblemNameParser.h"

constexpr int MAX_FILE_NAME_LENGTH = 255;
constexpr int BUFFER_LAST_CHARACTER_INDEX = MAX_FILE_NAME_LENGTH - 1;

unsigned int MCYear(std::filesystem::path file_path) {
  return MCYear(file_path.filename().string());
}
unsigned int MCYear(std::string problem_file_name) {
  char file_name[MAX_FILE_NAME_LENGTH];
  strncpy(file_name, problem_file_name.c_str(), MAX_FILE_NAME_LENGTH);
  file_name[BUFFER_LAST_CHARACTER_INDEX] = '\0';
  std::string prefix = strtok(file_name, "-");
  std::string year = strtok(nullptr, "-");
  unsigned int mc_year = std::stoi(year);
  return mc_year;
}
