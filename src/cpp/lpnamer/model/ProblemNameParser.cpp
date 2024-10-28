//
// Created by marechaljas on 02/05/2022.
//

#include <filesystem>
#include <boost/tokenizer.hpp>
#include "antares-xpansion/lpnamer/model/ProblemNameParser.h"

std::pair<unsigned int, unsigned int> McYearAndWeek(const std::filesystem::path &file_path) {
  return McYearAndWeek(file_path.filename().string());
}
std::pair<unsigned int, unsigned int> McYearAndWeek(const std::string &problem_file_name) {
  boost::tokenizer<> tok(problem_file_name);
  auto prefix = tok.begin();
  std::string str = *(++prefix);
  unsigned int mc_year = std::stoi(str);
  str = *(++prefix);
  unsigned int week = std::stoi(str);
  return {mc_year, week};
}
