#include "JsonXpansionReader.h"

#include <json/json.h>

#include <fstream>
#include <iostream>

JsonXpansionReader::JsonXpansionReader() : _lastIterNb(0) {}

void JsonXpansionReader::read(const std::filesystem::path& filename_p) {
  std::ifstream input_file_l(filename_p, std::ifstream::binary);

  Json::CharReaderBuilder builder_l;
  std::string errs;
  if (!parseFromStream(builder_l, input_file_l, &_input, &errs)) {
    std::cerr << errs << std::endl;
  }

  // save lastIteration
  _lastIterNb = 0;
  for (auto const& iterationId_cnt : _input["iterations"].getMemberNames()) {
    int iterNb_l = std::stoi(iterationId_cnt);
    _lastIterNb = (_lastIterNb < iterNb_l) ? iterNb_l : _lastIterNb;
  }
}

int JsonXpansionReader::getBestIteration() const {
  return _input["solution"]["iteration"].asInt();
}

int JsonXpansionReader::getLastIteration() const { return _lastIterNb; }

JsonXpansionReader::Point JsonXpansionReader::getSolutionPoint() const {
  Point x;
  for (auto const& name_l : _input["solution"]["values"].getMemberNames()) {
    x[name_l] = _input["solution"]["values"][name_l].asDouble();
  }

  return x;
}
