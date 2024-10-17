//
// Created by marechaljas on 02/05/2022.
//

#include <filesystem>
#include <boost/tokenizer.hpp>
#include "antares-xpansion/lpnamer/model/ProblemNameParser.h"

unsigned int MCYear(const std::filesystem::path &file_path) {
  return MCYear(file_path.filename().string());
}
unsigned int MCYear(const std::string &problem_file_name) {
  boost::tokenizer<> tok(problem_file_name);
  auto prefix = tok.begin();
  std::string year = *(++prefix);
  unsigned int mc_year = std::stoi(year);
  return mc_year;
}
